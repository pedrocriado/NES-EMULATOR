#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <ifaddrs.h>
#include <limits.h>
#include <net/if.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/uio.h>
#include <time.h>
#include <mdns.h>

#include "PB2.h"
#include "../Core/CPU6502.h"
#include "../Core/PPU.h"
#include "../Core/Cartridge.h"
#include "../Core/Controller.h"

static void PB2_rom_reset(PB2Rom* rom);
static int PB2_send_all(int fd, const uint8_t* data, size_t length);
static int PB2_recv(int fd, uint8_t* data, size_t length);
static int PB2_send_frame(int fd, uint8_t type, const uint8_t* data, uint32_t length);
static inline int PB2_send_error(int fd, const char* message);
static void PB2_sanitize_filename(const char* input, char* output, size_t cap);
static int PB2_mkdir(const char* path, mode_t mode);
static int PB2_ensure_upload_dir(void);
static int PB2_handle_rom_begin(PB2Rom* rom, const uint8_t* payload, uint32_t length);
static int PB2_finalize_rom(PB2Rom* sink, char* message, size_t message_cap,
                            char* rom_path, size_t rom_path_cap);
static void PB2_emulator_init(PB2* emu);
static void PB2_free(PB2* emu);
static void PB2_normalize_input(PB2* emu, uint8_t state);
static int PB2_emulator_load_rom(PB2* emu, const char* path);
static int PB2_read_header(int fd, uint8_t* type, uint32_t* length);
static int PB2_read_payload(int fd, uint32_t length, uint8_t** payload);
static int PB2_send_framebuffer(PB2Session* session);
static int PB2_stream_frames(PB2Session* session, uint8_t initial_input);
static int PB2_choose_ipv4(struct sockaddr_in* out_addr);
static void PB2_prepare_service(PB2Mdns* service);
static int PB2_answer(int sock, const struct sockaddr* from, size_t addrlen, uint16_t query_id,
                      uint16_t rtype, const mdns_string_t* question_name, mdns_record_t answer,
                      const mdns_record_t* additional, size_t additional_count, int unicast);
static int PB2_service_callback(int sock, const struct sockaddr* from, size_t addrlen,
                                mdns_entry_type_t entry, uint16_t query_id, uint16_t rtype,
                                uint16_t rclass, uint32_t ttl, const void* data, size_t size,
                                size_t name_offset, size_t name_length, size_t record_offset,
                                size_t record_length, void* userdata);
static void* pb2_mdns_thread(void* arg);
static int create_server_socket(void);
static void PB2_handle_connection(int client_fd, const struct sockaddr_in* addr);


static void PB2_rom_reset(PB2Rom* rom) 
{
    if(rom->fp) 
    {
        fflush(rom->fp);
        fclose(rom->fp);
    }
    memset(rom, 0, sizeof(*rom));
}

static int PB2_send_all(int fd, const uint8_t* data, size_t length) 
{
    size_t sent = 0;
    while(sent < length) 
    {
        ssize_t res = send(fd, data + sent, length - sent, 0);
        if(res < 0) {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            return -1;
        }
        if(res == 0)
            return -1;
        sent += res;
    }
    return 0;
}

static int PB2_recv(int fd, uint8_t* data, size_t length) 
{
    size_t read_total = 0;
    while(read_total < length) {
        ssize_t res = recv(fd, data + read_total, length - read_total, 0);
        if(res < 0) {
            if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            return -1;
        }
        if(res == 0)
            return 0;
        read_total += (size_t)res;
    }
    return 1;
}

static int PB2_send_frame(int fd, uint8_t type, const uint8_t* data, uint32_t length) {
    uint8_t header[PB2_PACKET_HEADER_SIZE];
    uint32_t be_len = htonl(length);
    header[0] = type;
    memcpy(&header[1], &be_len, sizeof(be_len));

    if(PB2_send_all(fd, header, sizeof(header)) < 0)
        return -1;
    if(length) 
        if(PB2_send_all(fd, data, length) < 0)
            return -1;

    return 0;
}

static inline int PB2_send_error(int fd, const char* message) 
{
    return PB2_send_frame(fd, PB2_MSG_ERROR, message, strlen(message));
}

static void PB2_sanitize_filename(const char* input, char* output, size_t cap) 
{
    size_t out_len = 0;
    for(size_t i = 0; input && input[i] && out_len + 1 < cap; ++i) 
    {
        char c = input[i];
        if(c == '/' || c == '\\')
            continue;
        if(isalnum(c) || c == '-' || c == '_' || c == '.') {
            output[out_len++] = (char)c;
        }
    }
    if(out_len == 0) 
    {
        strncpy(output, "uploaded_rom.nes", cap - 1);
        output[cap - 1] = '\0';
        return;
    }
    output[out_len] = '\0';
}

static int PB2_mkdir(const char* path, mode_t mode) 
{
    if(!path || !*path)
        return -1;

    char tmp[1024];
    size_t len = strnlen(path, sizeof(tmp));
    if(len == 0 || len >= sizeof(tmp))
        return -1;

    memcpy(tmp, path, len);
    tmp[len] = '\0';

    for(char* p = tmp + 1; *p; ++p) 
    {
        if(*p == '/') {
            *p = '\0';
            if(mkdir(tmp, mode) < 0)
                return -1;
            *p = '/';
        }
    }

    if(mkdir(tmp, mode) < 0)
        return -1;
    return 0;
}

static int PB2_ensure_upload_dir() 
{
    struct stat st;
    if(stat(PB2_ROM_UPLOAD_DIR, &st) == 0) 
        return S_ISDIR(st.st_mode) ? 0 : -1;
    
    if(PB2_mkdir(PB2_ROM_UPLOAD_DIR, 0755) < 0)
        return -1;
    return 0;
}

static int PB2_handle_rom_begin(PB2Rom* rom, const uint8_t* payload, uint32_t length) 
{
    if(length < sizeof(uint32_t) + 1)
        return -1;

    PB2_rom_reset(rom);

    uint32_t total_be = 0;
    memcpy(&total_be, payload, sizeof(uint32_t));
    uint32_t total_size = ntohl(total_be);

    size_t name_length = length - sizeof(uint32_t);
    if(name_length >= 256)
        name_length = 255;

    char raw_name[256];
    memcpy(raw_name, payload + sizeof(uint32_t), name_length);
    raw_name[name_length] = '\0';

    char safe_name[256];
    PB2_sanitize_filename(raw_name, safe_name, sizeof(safe_name));

    if(PB2_ensure_upload_dir() < 0) 
    {
        perror("[PB2] Failed to prepare upload directory");
        return -1;
    }

    char full_path[sizeof(rom->path)];
    snprintf(full_path, sizeof(full_path), "%s/%s", PB2_ROM_UPLOAD_DIR, safe_name);

    FILE* fp = fopen(full_path, "wb");
    if(!fp) {
        perror("[PB2] fopen");
        return -1;
    }

    PB2_rom_reset(rom);
    rom->fp = fp;
    rom->expected_size = total_size;
    rom->received_size = 0;
    strncpy(rom->path, full_path, sizeof(rom->path) - 1);
    rom->path[sizeof(rom->path) - 1] = '\0';
    printf("[PB2] Receiving ROM '%s' (%u bytes) -> %s\n", safe_name, total_size, rom->path);
    return 0;
}

static int PB2_finalize_rom(PB2Rom* sink, char* message, size_t message_cap,
                            char* rom_path, size_t rom_path_cap) 
{
    if(!sink->fp)
        return -1;
    fflush(sink->fp);
    fclose(sink->fp);
    sink->fp = NULL;

    int ok = (!sink->expected_size) || (sink->received_size == sink->expected_size);
    char stored_path[sizeof(sink->path)];
    strncpy(stored_path, sink->path, sizeof(stored_path) - 1);
    stored_path[sizeof(stored_path) - 1] = '\0';

    if(rom_path && rom_path_cap) {
        strncpy(rom_path, stored_path, rom_path_cap - 1);
        rom_path[rom_path_cap - 1] = '\0';
    }

    int n;
    if(ok) {
        n = snprintf(message, message_cap, "ROM stored at %s (%u bytes)",
                     stored_path, sink->received_size);
    } else {
        n = snprintf(message, message_cap, "ROM stored at %s (expected %u, got %u)",
                     stored_path, sink->expected_size, sink->received_size);
    }
    if(n < 0)
        message[0] = '\0';
    PB2_rom_reset(sink);
    return ok ? 0 : -1;
}

static void PB2_emulator_init(PB2* emu) 
{
    memset(emu, 0, sizeof(*emu));
    Cartridge_reset(&emu->cart);

    emu->cpu.ppu = &emu->ppu;
    emu->cpu.cart = &emu->cart;
    emu->cpu.controller[0] = &emu->controllers[0];
    emu->cpu.controller[1] = &emu->controllers[1];

    Controller_init(&emu->controllers[0]);
    Controller_init(&emu->controllers[1]);
    emu->ppu.cart = &emu->cart;
}

static void PB2_free(PB2* emu) {
    PPU_free(&emu->ppu);
    Cartridge_reset(&emu->cart);
    emu->cart_loaded = false;
}

static void PB2_normalize_input(PB2* emu, uint8_t state) 
{
    if((state & 0xC0) == 0xC0)
        state &= ~0xC0;
    if((state & 0x30) == 0x30)
        state &= ~0x30;
    emu->controllers[0].state = state;
}

static int PB2_emulator_load_rom(PB2* emu, const char* path) 
{
    if(!path || !path[0])
        return -1;

    memset(emu->cpu.ram, 0, sizeof(emu->cpu.ram));
    Controller_init(&emu->controllers[0]);
    Controller_init(&emu->controllers[1]);

    PPU_free(&emu->ppu);

    Cartridge_load(&emu->cart, path);

    emu->cpu.cart = &emu->cart;
    emu->ppu.cart = &emu->cart;

    PPU_init(&emu->ppu);
    emu->ppu.cart = &emu->cart;
    switch(emu->cart.mapper.tv) 
    {
        case PAL:
        case DENDY:
            emu->ppu.scanLinesPerFame = 312;
            break;
        default:
            emu->ppu.scanLinesPerFame = ALL_SCANLINES;
            break;
    }

    CPU_init(&emu->cpu);
    emu->cart_loaded = true;
    return 0;
}

static int PB2_read_header(int fd, uint8_t* type, uint32_t* length) 
{
    uint8_t header[PB2_PACKET_HEADER_SIZE];
    int res = PB2_recv(fd, header, sizeof(header));
    if(res <= 0)
        return res;
    *type = header[0];
    uint32_t be_len = 0;
    memcpy(&be_len, &header[1], sizeof(be_len));
    *length = ntohl(be_len);
    return 1;
}

static int PB2_read_payload(int fd, uint32_t length, uint8_t** payload) 
{
    if(length == 0) {
        *payload = NULL;
        return 1;
    }
    uint8_t* data = malloc(length);
    if(!data)
        return -1;
    int res = PB2_recv(fd, data, length);
    if(res <= 0) {
        free(data);
        return res;
    }
    *payload = data;
    return 1;
}

static int PB2_send_framebuffer(PB2Session* session) 
{
    if(!session || !session->emu.ppu.screen || !session->udp_ready || session->udp_fd < 0)
        return -1;

    uint8_t header[PB2_PACKET_HEADER_SIZE];
    uint32_t payload_len = PB2_FRAME_PAYLOAD_SIZE;
    uint32_t be_len = htonl(payload_len);
    header[0] = PB2_MSG_FRAME;
    memcpy(&header[1], &be_len, sizeof(be_len));
    uint32_t frame_id_be = htonl(session->next_frame_id++);

    struct iovec iov[3];
    iov[0].iov_base = header;
    iov[0].iov_len = sizeof(header);
    iov[1].iov_base = &frame_id_be;
    iov[1].iov_len = sizeof(frame_id_be);
    iov[2].iov_base = session->emu.ppu.screen;
    iov[2].iov_len = PB2_FRAME_PIXELS;

    struct msghdr msg = {0};
    msg.msg_name = &session->udp_peer;
    msg.msg_namelen = sizeof(session->udp_peer);
    msg.msg_iov = iov;
    msg.msg_iovlen = 3;

    sendmsg(session->udp_fd, &msg, 0);

    return 0;
}

static int PB2_stream_frames(PB2Session* session, uint8_t initial_input) 
{
    if(!session)
        return -1;
    if(!session->udp_ready) {
        PB2_send_error(session->fd, "UDP port not set");
        return -1;
    }

    printf("[PB2] Starting frame stream\n");
    PB2_normalize_input(&session->emu, initial_input);

    TVSystem tv = session->emu.cart.mapper.tv;
    long target_frame_ns = (tv == PAL || tv == DENDY) ? 20000000L : 16666667L;

    struct timespec fps_start;
    clock_gettime(CLOCK_MONOTONIC, &fps_start);
    uint32_t fps_frames = 0;

    while(g_keep_running) {
        struct timespec frame_start;
        clock_gettime(CLOCK_MONOTONIC, &frame_start);

        session->emu.ppu.frameComple = false;
        while(!session->emu.ppu.frameComple) {
            PPU_clock(&session->emu.ppu);
            PPU_clock(&session->emu.ppu);
            PPU_clock(&session->emu.ppu);
            session->emu.cpu.nmiLine = session->emu.ppu.nmi;
            CPU_clock(&session->emu.cpu);
        }

        PB2_send_framebuffer(session);

        fps_frames++;
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long produced_ns = (now.tv_sec - frame_start.tv_sec) * 1000000000L +
                           (now.tv_nsec - frame_start.tv_nsec);
        if(produced_ns < target_frame_ns) {
            struct timespec sleep_time = {
                .tv_sec = 0,
                .tv_nsec = target_frame_ns - produced_ns
            };
            nanosleep(&sleep_time, NULL);
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(session->fd, &readfds);
        struct timeval timeout = {0, 0};
        int ready = select(session->fd + 1, &readfds, NULL, NULL, &timeout);
        if(ready < 0) 
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        if(ready == 0)
            continue;

        uint8_t type = 0;
        uint32_t length = 0;
        int hdr = PB2_read_header(session->fd, &type, &length);
        if(hdr <= 0) 
        {
            printf("[PB2] Connection closed while waiting for input (%d)\n", hdr);
            return hdr;
        }

        uint8_t* payload = NULL;
        if(PB2_read_payload(session->fd, length, &payload) <= 0) {
            free(payload);
            printf("[PB2] Failed to read payload of type 0x%02X\n", type);
            return -1;
        }

        if(type == PB2_MSG_INPUT) 
        {
            uint8_t state = (payload && length > 0) ? payload[0] : 0;
            PB2_normalize_input(&session->emu, state);
            free(payload);
            continue;
        } 
        else 
        {
            printf("[PB2] Unexpected message 0x%02X during stream\n", type);
            free(payload);
            return -1;
        }
    }

    return 0;
}

static int PB2_choose_ipv4(struct sockaddr_in* out_addr) 
{
    struct ifaddrs* list;
    if(getifaddrs(&list) < 0)
        return -1;

    int found = -1;
    for(struct ifaddrs* net = list; net; net = net->ifa_next) {
        if(!net->ifa_addr || net->ifa_addr->sa_family != AF_INET)
            continue;
        if(strcmp(PB2_MDNS_DEFAULT_IFACE, net->ifa_name) != 0)
            continue;

        struct sockaddr_in* addr = (struct sockaddr_in*)net->ifa_addr;
        if(addr->sin_addr.s_addr == htonl(INADDR_LOOPBACK))
            continue;

        memcpy(out_addr, addr, sizeof(*out_addr));
        out_addr->sin_port = 0;
        found = 0;
        break;
    }

    freeifaddrs(list);
    return found;
}

static void PB2_prepare_service(PB2Mdns* service) 
{
    memset(service, 0, sizeof(*service));

    gethostname(service->hostname, sizeof(service->hostname));
    printf("[PocketBeagle 2] Hostname: %s\n", service->hostname);
    snprintf(service->hostname_fqdn, sizeof(service->hostname_fqdn), "%s.local.", service->hostname);
    snprintf(service->service_type, sizeof(service->service_type), "%s.local.", PB2_SERVICE_TYPE);
    snprintf(service->service_instance, sizeof(service->service_instance), "%s.%s.local.", PB2_SERVICE_NAME, PB2_SERVICE_TYPE);

    strncpy(service->txt_board_key, "board", sizeof(service->txt_board_key) - 1);
    strncpy(service->txt_board_value, "PocketBeagle2", sizeof(service->txt_board_value) - 1);
    strncpy(service->txt_role_key, "role", sizeof(service->txt_role_key) - 1);
    strncpy(service->txt_role_value, "frame-server", sizeof(service->txt_role_value) - 1);

    service->hostname_label.str = service->hostname;
    service->hostname_label.length = strlen(service->hostname);
    service->hostname_fqdn_label.str = service->hostname_fqdn;
    service->hostname_fqdn_label.length = strlen(service->hostname_fqdn);
    service->service_label.str = service->service_type;
    service->service_label.length = strlen(service->service_type);
    service->service_instance_label.str = service->service_instance;
    service->service_instance_label.length = strlen(service->service_instance);
    service->dns_sd_label.str = PB2_DNS_SN_NAME;
    service->dns_sd_label.length = strlen(PB2_DNS_SN_NAME);

    if(PB2_choose_ipv4(&service->address_ipv4) < 0) 
    {
        service->address_ipv4.sin_family = AF_INET;
        service->address_ipv4.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        printf("[PB2] Falling back to loopback for mDNS\n");
    } 
    else 
    {
        char addrbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &service->address_ipv4.sin_addr, addrbuf, sizeof(addrbuf));
        printf("[PB2] Using IPv4 interface %s (%s)\n", PB2_MDNS_DEFAULT_IFACE, addrbuf);
    }
    service->address_ipv4.sin_port = 0;
    service->port = PB2_TCP_PORT;

    service->record_dns_sd = (mdns_record_t){
        .name = service->dns_sd_label,
        .type = MDNS_RECORDTYPE_PTR,
        .data.ptr.name = service->service_label,
        .ttl = 120
    };

    service->record_ptr = (mdns_record_t){
        .name = service->service_label,
        .type = MDNS_RECORDTYPE_PTR,
        .data.ptr.name = service->service_instance_label,
        .ttl = 120
    };

    service->record_srv = (mdns_record_t){
        .name = service->service_instance_label,
        .type = MDNS_RECORDTYPE_SRV,
        .data.srv.name = service->hostname_fqdn_label,
        .data.srv.port = (uint16_t)service->port,
        .data.srv.priority = 0,
        .data.srv.weight = 0,
        .ttl = 120
    };

    service->record_a = (mdns_record_t){
        .name = service->hostname_fqdn_label,
        .type = MDNS_RECORDTYPE_A,
        .data.a.addr = service->address_ipv4,
        .ttl = 120
    };

    service->txt_record[0] = (mdns_record_t){
        .name = service->service_instance_label,
        .type = MDNS_RECORDTYPE_TXT,
        .data.txt.key = { service->txt_board_key, strlen(service->txt_board_key) },
        .data.txt.value = { service->txt_board_value, strlen(service->txt_board_value) },
        .ttl = 120
    };

    service->txt_record[1] = (mdns_record_t){
        .name = service->service_instance_label,
        .type = MDNS_RECORDTYPE_TXT,
        .data.txt.key = { service->txt_role_key, strlen(service->txt_role_key) },
        .data.txt.value = { service->txt_role_value, strlen(service->txt_role_value) },
        .ttl = 120
    };
}

static int PB2_answer(int sock, const struct sockaddr* from, size_t addrlen, uint16_t query_id,
                      uint16_t rtype, const mdns_string_t* question_name, mdns_record_t answer,
                      const mdns_record_t* additional, size_t additional_count, int unicast) 
{
    if(unicast) return mdns_query_answer_unicast(sock, from, addrlen, g_mdns_send_buffer,
                                         sizeof(g_mdns_send_buffer), query_id, rtype,
                                         question_name->str, question_name->length,
                                         answer, NULL, 0, additional, additional_count);
    return mdns_query_answer_multicast(sock, g_mdns_send_buffer, sizeof(g_mdns_send_buffer),
                                       answer, NULL, 0, additional, additional_count);
}

static int PB2_service_callback(int sock, const struct sockaddr* from, size_t addrlen,
                                mdns_entry_type_t entry, uint16_t query_id, uint16_t rtype,
                                uint16_t rclass, uint32_t ttl, const void* data, size_t size,
                                size_t name_offset, size_t name_length, size_t record_offset,
                                size_t record_length, void* userdata) 
{
    if(entry != MDNS_ENTRYTYPE_QUESTION)
        return 0;

    PB2Mdns* service = userdata;
    char name_buffer[256];
    size_t offset = name_offset;
    mdns_string_t name = mdns_string_extract(data, size, &offset, name_buffer, sizeof(name_buffer) - 1);
    if(!name.length)
        return 0;
    name_buffer[name.length] = '\0';
    name.str = name_buffer;

    int unicast = (rclass & MDNS_UNICAST_RESPONSE) ? 1 : 0;

    mdns_record_t additional[4];
    additional[0] = service->record_srv;
    additional[1] = service->record_a;
    additional[2] = service->txt_record[0];
    additional[3] = service->txt_record[1];

    if((rtype == MDNS_RECORDTYPE_PTR) && strcasecmp(name_buffer, service->dns_sd_label.str) == 0) 
    {
        PB2_answer(sock, from, addrlen, query_id, rtype, &name, service->record_dns_sd,
                   &service->record_ptr, 1, unicast);
        return 0;
    }

    if((rtype == MDNS_RECORDTYPE_PTR) && strcasecmp(name_buffer, service->service_label.str) == 0) 
    {
        PB2_answer(sock, from, addrlen, query_id, rtype, &name, service->record_ptr,
                   additional, 4, unicast);
        return 0;
    }

    if(strcasecmp(name_buffer, service->service_instance_label.str) == 0) 
    {
        if(rtype == MDNS_RECORDTYPE_SRV)
            PB2_answer(sock, from, addrlen, query_id, rtype, &name, service->record_srv,
                       &service->record_a, 1, unicast);
        else if(rtype == MDNS_RECORDTYPE_TXT)
            PB2_answer(sock, from, addrlen, query_id, rtype, &name, service->txt_record[0],
                       service->txt_record, 2, unicast);
        return 0;
    }

    if((rtype == MDNS_RECORDTYPE_A) &&
       strcasecmp(name_buffer, service->hostname_fqdn_label.str) == 0) 
    {
        PB2_answer(sock, from, addrlen, query_id, rtype, &name, service->record_a,
                   NULL, 0, unicast);
        return 0;
    }

    return 0;
}

static void* pb2_mdns_thread(void* arg) {
    PB2Mdns* service = arg;

    struct sockaddr_in sock_addr = service->address_ipv4;
    if(sock_addr.sin_family != AF_INET) {
        memset(&sock_addr, 0, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    } 
    else if(sock_addr.sin_addr.s_addr == 0)
        sock_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sock_addr.sin_port = htons(MDNS_PORT);

    char iface_addr[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &sock_addr.sin_addr, iface_addr, sizeof(iface_addr));
    printf("[PB2] Binding mDNS socket to %s\n", iface_addr);

    int sock = mdns_socket_open_ipv4(&sock_addr);
    if(sock < 0)
        return NULL;

    mdns_record_t announce_additional[4] = {
        service->record_srv,
        service->record_a,
        service->txt_record[0],
        service->txt_record[1]
    };

    mdns_announce_multicast(sock, g_mdns_send_buffer, sizeof(g_mdns_send_buffer),
                            service->record_ptr, NULL, 0,
                            announce_additional, sizeof(announce_additional)/sizeof(announce_additional[0]));
    mdns_announce_multicast(sock, g_mdns_send_buffer, sizeof(g_mdns_send_buffer),
                            service->record_dns_sd, NULL, 0, NULL, 0);

    uint8_t buffer[2048];
    while(g_keep_running) 
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        struct timeval timeout = { .tv_sec = 0, .tv_usec = 200000 };
        int res = select(sock + 1, &readfds, NULL, NULL, &timeout);
        if(res > 0 && FD_ISSET(sock, &readfds))
            mdns_socket_listen(sock, buffer, sizeof(buffer), PB2_service_callback, service);
    }

    mdns_goodbye_multicast(sock, g_mdns_send_buffer, sizeof(g_mdns_send_buffer),
                           service->record_ptr, NULL, 0,
                           announce_additional, sizeof(announce_additional)/sizeof(announce_additional[0]));
    mdns_goodbye_multicast(sock, g_mdns_send_buffer, sizeof(g_mdns_send_buffer),
                           service->record_dns_sd, NULL, 0, NULL, 0);

    mdns_socket_close(sock);
    return NULL;
}

static int create_server_socket() 
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0)
        return -1;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PB2_TCP_PORT);

    if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) 
    {
        close(fd);
        return -1;
    }

    if(listen(fd, 1) < 0) 
    {
        close(fd);
        return -1;
    }

    return fd;
}

static void PB2_handle_connection(int client_fd, const struct sockaddr_in* addr) {
    char remote[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, remote, sizeof(remote));
    printf("[PB2] Client connected from %s:%u\n", remote, ntohs(addr->sin_port));

    const char* banner = "PB2 server ready\n";
    PB2_send_all(client_fd, banner, strlen(banner));

    PB2Session session = (PB2Session)
    {
        .next_frame_id = 0, .client_addr = *addr,
        .udp_fd = -1, .udp_ready = false, 
        .fd = client_fd
    };
    
    PB2_emulator_init(&session.emu);

    bool exit_loop = false;
    while(g_keep_running && !exit_loop) 
    {
        uint8_t type = 0;
        uint32_t payload_len = 0;
        int header_res = PB2_read_header(client_fd, &type, &payload_len);
        if(header_res <= 0)
            break;

        if(type == PB2_MSG_ROM_CHUNK) {
            if(payload_len == 0 || session.rom_sink.fp) {
                uint32_t remaining = payload_len;
                uint8_t buffer[PB2_ROM_BUFFER_SIZE];
                bool error = false;
                while(remaining > 0) {
                    size_t chunk = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
                    int res = PB2_recv(client_fd, buffer, chunk);
                    if(res <= 0 || !session.rom_sink.fp) {
                        error = true;
                        break;
                    }
                    size_t written = fwrite(buffer, 1, chunk, session.rom_sink.fp);
                    if(written != chunk) {
                        error = true;
                        break;
                    }
                    session.rom_sink.received_size += (uint32_t)written;
                    remaining -= (uint32_t)chunk;
                }
                if(error) {
                    PB2_send_error(client_fd, "ROM chunk write failed");
                    PB2_rom_reset(&session.rom_sink);
                    break;
                }
                continue;
            } else {
                PB2_send_error(client_fd, "ROM chunk write failed");
                PB2_rom_reset(&session.rom_sink);
                break;
            }
        }

        uint32_t limit = 0;
        switch(type) 
        {
            case PB2_MSG_ROM_BEGIN:
                limit = 512;
                break;
            case PB2_MSG_STREAM_BEGIN:
            case PB2_MSG_INPUT:
                limit = 1;
                break;
            case PB2_MSG_UDP_PORT:
                limit = sizeof(uint16_t);
                break;
            default:
                break;
        }
        if(limit && payload_len > limit) {
            uint32_t remaining = payload_len;
            uint8_t scratch[256];
            while(remaining) {
                size_t chunk = remaining < sizeof(scratch) ? remaining : sizeof(scratch);
                int res = PB2_recv(client_fd, scratch, chunk);
                if(res <= 0)
                    break;
                remaining -= (uint32_t)chunk;
            }
            PB2_send_error(client_fd, "Payload too large");
            continue;
        }

        uint8_t* payload = NULL;
        char msg[256];
        if(PB2_read_payload(client_fd, payload_len, &payload) <= 0) 
        {
            free(payload);
            break;
        }

        switch(type) 
        {
            case PB2_MSG_ROM_BEGIN:
                if(PB2_handle_rom_begin(&session.rom_sink, payload, payload_len) == 0)
                    PB2_send_frame(client_fd, PB2_MSG_ACK, "ROM upload initialized", strlen("ROM upload initialized"));
                else
                    PB2_send_error(client_fd, "ROM begin failed");
                break;
            case PB2_MSG_ROM_END:
                char rom_path[sizeof(session.rom_sink.path)];
                if(PB2_finalize_rom(&session.rom_sink, msg, sizeof(msg),
                                    rom_path, sizeof(rom_path)) == 0) 
                {
                    if(PB2_emulator_load_rom(&session.emu, rom_path) == 0) 
                    {
                        session.next_frame_id = 0;
                        PB2_send_frame(client_fd, PB2_MSG_ROM_RESULT, msg, strlen(msg));
                    } 
                    else 
                        PB2_send_error(client_fd, "Failed to load ROM");
                } 
                else 
                {
                    const char* err = msg[0] ? msg : "ROM finalize failed";
                    PB2_send_error(client_fd, err);
                }
                break;
            case PB2_MSG_STREAM_BEGIN:
                uint8_t start_state = (payload && payload_len) ? payload[0] : 0;
                if(!session.emu.cart_loaded) 
                    PB2_send_error(client_fd, "No ROM loaded");
                else 
                {
                    free(payload);
                    payload = NULL;
                    if(PB2_stream_frames(&session, start_state) < 0)
                        printf("[PB2] Streaming stopped\n");
                    exit_loop = true;
                }
                break;
            case PB2_MSG_INPUT:
                PB2_normalize_input(&session.emu, payload[0]);
                break;
            case PB2_MSG_UDP_PORT:
                if(payload_len != sizeof(uint16_t)) 
                {
                    PB2_send_error(client_fd, "Invalid UDP port payload");
                    break;
                }
                uint16_t be_port;
                memcpy(&be_port, payload, sizeof(be_port));

                if(session.udp_fd >= 0) {
                    close(session.udp_fd);
                    session.udp_fd = -1;
                }
                session.udp_ready = false;
                int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
                if(udp_fd < 0) 
                {
                    PB2_send_error(client_fd, "Failed to open UDP socket");
                    break;
                }

                session.udp_fd = udp_fd;
                session.udp_peer = session.client_addr;
                session.udp_peer.sin_port = be_port;
                session.udp_ready = true;

                PB2_send_frame(client_fd, PB2_MSG_ACK, "udp-ready", strlen("udp-ready"));
                break;
            default:
                PB2_send_error(client_fd, "Unknown message type");
                break;
        }

        free(payload);
    }

    PB2_rom_reset(&session.rom_sink);
    PB2_free(&session.emu);
    if(session.udp_fd >= 0) {
        close(session.udp_fd);
        session.udp_fd = -1;
    }
    session.udp_ready = false;
    printf("[PB2] Client disconnected\n");
}

int main() 
{
    PB2Mdns service;
    PB2_prepare_service(&service);

    pthread_t mdns_thread;
    if(pthread_create(&mdns_thread, NULL, pb2_mdns_thread, &service) != 0)
        return 1;

    int server_fd = create_server_socket();
    if(server_fd < 0) 
    {
        g_keep_running = 0;
        pthread_join(mdns_thread, NULL);
        return 1;
    }

    printf("[PB2] Listening on TCP port %d...\n", PB2_TCP_PORT);

    while(g_keep_running) 
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);

        PB2_handle_connection(client_fd, &client_addr);
        close(client_fd);
    }

    close(server_fd);
    g_keep_running = 0;
    pthread_join(mdns_thread, NULL);
    return 0;
}