#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mdns.h>

#include "LinuxRender.h"
#include "../Desktop/Graphics.h"
#include "../Core/Controller.h"

static int Linux_choose_ipv4(struct sockaddr_in* out_addr);
static int Linux_send_all(int sock, const uint8_t* data, size_t length);
static int Linux_recv_all(int sock, void* buffer, size_t length);
static int Linux_send_frame(int sock, uint8_t type, const uint8_t* data, uint32_t length);
static int Linux_wait_for_frame(int sock, uint8_t expected_type);
static int Linux_send_rom(int sock, const char* rom_path);
static int Linux_create_udp_socket(uint16_t* out_port);
static int Linux_discover(struct sockaddr_in* out_addr);
static void Linux_keyboard_input(JoyPad* ctrl);
static int Linux_discovery_cb(int sock, const struct sockaddr* from, size_t addrlen,
                              mdns_entry_type_t entry, uint16_t query_id, uint16_t rtype,
                              uint16_t rclass, uint32_t ttl, const void* data, size_t size,
                              size_t name_offset, size_t name_length, size_t record_offset,
                              size_t record_length, void* userdata);

static int Linux_choose_ipv4(struct sockaddr_in* out_addr) 
{
    struct ifaddrs* networkList;
    if(getifaddrs(&networkList) < 0)
        return -1;

    struct ifaddrs* network = networkList;
    while(network) {
        if(!network->ifa_addr 
        || !(network->ifa_flags & (IFF_UP | IFF_LOOPBACK))
        || (network->ifa_addr->sa_family != AF_INET)
        || (strcmp("eth1", network->ifa_name)))
        {
            network = network->ifa_next;
            continue;
        }

        struct sockaddr_in* addr = (struct sockaddr_in*)network->ifa_addr;
        if(addr->sin_addr.s_addr == htonl(INADDR_LOOPBACK))
        {
            network = network->ifa_next;
            continue;
        }

        memcpy(out_addr, addr, sizeof(*out_addr));
        out_addr->sin_port = 0;

        freeifaddrs(networkList);
        char addrbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &out_addr->sin_addr, addrbuf, sizeof(addrbuf));
        return 0;
    }

    freeifaddrs(networkList);
    return -1;
}

static int Linux_send_all(int sock, const uint8_t* data, size_t length) 
{
    size_t sent = 0;
    while(sent < length) {
        ssize_t res = send(sock, data + sent, length - sent, 0);
        if(res < 0) {
            if(errno == EINTR)
                continue;
            return -1;
        }
        if(res == 0)
            return -1;
        sent += (size_t)res;
    }
    return 0;
}

static int Linux_recv_all(int sock, void* buffer, size_t length) 
{
    uint8_t* data = buffer;
    size_t total = 0;
    while(total < length) {
        ssize_t res = recv(sock, data + total, length - total, 0);
        if(res < 0) 
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        if(res == 0)
            return 0;
        total += res;
    }
    return 1;
}

static int Linux_send_frame(int sock, uint8_t type, const uint8_t* data, uint32_t length) 
{
    uint8_t header[LINUX_PACKET_HEADER_SIZE];
    uint32_t be_len = htonl(length);
    header[0] = type;
    memcpy(&header[1], &be_len, sizeof(be_len));
    if(Linux_send_all(sock, header, sizeof(header)) < 0)
        return -1;
    if(length) 
        if(Linux_send_all(sock, data, length) < 0)
            return -1;
    return 0;
}

static int Linux_recv_frame(int sock, uint8_t* type, uint8_t** payload, uint32_t* length) 
{
    uint8_t header[LINUX_PACKET_HEADER_SIZE];
    int res = Linux_recv_all(sock, header, sizeof(header));
    if(res <= 0)
        return res;

    uint32_t len;
    memcpy(&len, &header[1], sizeof(len));
    len = ntohl(len);

    *type = header[0];
    *length = len;
    *payload = NULL;

    if(len > 0) 
    {
        uint8_t* data = malloc(len + 1);
        if(!data)
            return -1;
        int payload_res = Linux_recv_all(sock, data, len);
        if(payload_res <= 0) 
        {
            free(data);
            return payload_res;
        }
        data[len] = '\0';
        *payload = data;
    }
    return 1;
}

static int Linux_wait_for_frame(int sock, uint8_t expected_type) 
{
    while(1) {
        uint8_t type = 0;
        uint8_t* payload = NULL;
        uint32_t length = 0;
        int res = Linux_recv_frame(sock, &type, &payload, &length);
        if(res <= 0) {
            free(payload);
            return res;
        }
        
        free(payload);
        if(type == PB2_MSG_ERROR)
            return -1;
        
        if(type == expected_type)
            return 1;
    }
}

static int Linux_send_rom(int sock, const char* rom_path) 
{
    struct stat st;
    if(stat(rom_path, &st) < 0) 
    {
        perror("[Linux] stat");
        return -1;
    }
    if(st.st_size > UINT32_MAX) 
    {
        printf("[Linux] ROM too large to send\n");
        return -1;
    }

    FILE* fp = fopen(rom_path, "rb");
    if(!fp) {
        perror("[Linux] fopen");
        return -1;
    }

    const char* base = strrchr(rom_path, '/');
    base = base ? base + 1 : rom_path;
    size_t name_len = strnlen(base, 512 - sizeof(uint32_t));

    uint8_t begin_payload[512];
    uint32_t total_size = (uint32_t)st.st_size;
    uint32_t total_be = htonl(total_size);
    memcpy(begin_payload, &total_be, sizeof(total_be));
    memcpy(begin_payload + sizeof(total_be), base, name_len);

    printf("[Linux] Sending ROM '%.*s' (%u bytes)\n", (int)name_len, base, total_size);
    if(Linux_send_frame(sock, PB2_MSG_ROM_BEGIN, begin_payload,
                      (uint32_t)(sizeof(total_be) + name_len)) < 0) {
        fclose(fp);
        return -1;
    }
    if(Linux_wait_for_frame(sock, PB2_MSG_ACK) <= 0) {
        fclose(fp);
        return -1;
    }

    uint8_t buffer[LINUX_ROM_BUFFER_SIZE];
    size_t read_bytes;
    while((read_bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if(Linux_send_frame(sock, PB2_MSG_ROM_CHUNK, buffer, (uint32_t)read_bytes) < 0) {
            fclose(fp);
            return -1;
        }
    }
    int read_error = ferror(fp);
    fclose(fp);
    if(read_error) 
    {
        printf("[Linux] Failed to read ROM data\n");
        return -1;
    }

    if(Linux_send_frame(sock, PB2_MSG_ROM_END, NULL, 0) < 0)
        return -1;
    return Linux_wait_for_frame(sock, PB2_MSG_ROM_RESULT);
}

static int Linux_create_udp_socket(uint16_t* out_port) 
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0) {
        perror("[Linux] socket");
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = 0;
    if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[Linux] bind");
        close(fd);
        return -1;
    }
    socklen_t len = sizeof(addr);
    if(getsockname(fd, (struct sockaddr*)&addr, &len) < 0) {
        perror("[Linux] getsockname");
        close(fd);
        return -1;
    }
    if(out_port)
        *out_port = ntohs(addr.sin_port);
    return fd;
}

static int Linux_discovery_cb(int sock, const struct sockaddr* from, size_t addrlen,
                              mdns_entry_type_t entry, uint16_t query_id, uint16_t rtype,
                              uint16_t rclass, uint32_t ttl, const void* data, size_t size,
                              size_t name_offset, size_t name_length, size_t record_offset,
                              size_t record_length, void* userdata) 
{
    DiscoveryState* state = userdata;

    char name_buffer[256];
    size_t offset = name_offset;
    mdns_string_t name = mdns_string_extract(data, size, &offset, name_buffer, sizeof(name_buffer) - 1);
    name_buffer[name.length] = '\0';

    if(rtype == MDNS_RECORDTYPE_PTR) 
    {
        char target[256];
        mdns_string_t ptr = mdns_record_parse_ptr(data, size, record_offset, record_length,
                                                  target, sizeof(target) - 1);
        target[ptr.length] = '\0';
        if(strcasecmp(target, state->instance_name) == 0)
            state->have_ptr = 1;
    } 
    else if((rtype == MDNS_RECORDTYPE_SRV) && strcasecmp(name_buffer, state->instance_name) == 0) 
    {
        char target[256];
        mdns_record_srv_t srv = mdns_record_parse_srv(data, size, record_offset, record_length,
                                                      target, sizeof(target) - 1);
        target[srv.name.length] = '\0';

        size_t cap = sizeof(state->target_host);
        size_t len = (srv.name.length < cap - 1) ? srv.name.length : (cap - 1);
        memcpy(state->target_host, target, len);

        state->target_host[len] = '\0';
        state->port = srv.port;
        state->have_srv = 1;
    } 
    else if(rtype == MDNS_RECORDTYPE_A) 
    {
        if(state->target_host[0] && strcasecmp(name_buffer, state->target_host) != 0)
            return 0;
        struct sockaddr_in addr;
        mdns_record_parse_a(data, size, record_offset, record_length, &addr);
        state->address = addr;
        state->have_a = 1;
    }

    return 0;
}

static int Linux_discover(struct sockaddr_in* out_addr) 
{
    struct sockaddr_in iface_addr;
    struct sockaddr_in* bind_iface;
    if(Linux_choose_ipv4(&iface_addr) == 0)
        bind_iface = &iface_addr;
    else
        printf("[Linux] Falling back to default interface for mDNS queries\n");

    int sock = mdns_socket_open_ipv4(bind_iface);
    if(sock < 0)
        return -1;

    DiscoveryState state = {0};
    snprintf(state.service_type, sizeof(state.service_type), "%s.local.", PB2_SERVICE_TYPE);
    snprintf(state.instance_name, sizeof(state.instance_name), "%s.%s.local.",
             PB2_SERVICE_NAME, PB2_SERVICE_TYPE);

    mdns_query_t query;
    query.type = MDNS_RECORDTYPE_PTR;
    query.name = state.service_type;
    query.length = strlen(state.service_type);

    uint8_t buffer[2048];
    int query_id = mdns_multiquery_send(sock, &query, 1, buffer, sizeof(buffer), 0);

    for(int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        if(state.have_srv && state.have_a)
            break;

        fd_set readfs;
        FD_ZERO(&readfs);
        FD_SET(sock, &readfs);
        struct timeval timeout = { .tv_sec = 0, .tv_usec = 200000 };
        int ready = select(sock + 1, &readfs, NULL, NULL, &timeout);
        if(ready > 0 && FD_ISSET(sock, &readfs)) 
            mdns_query_recv(sock, buffer, sizeof(buffer), Linux_discovery_cb, &state, query_id);
    }

    mdns_socket_close(sock);

    if(!(state.have_ptr && state.have_srv && state.have_a)) 
    {
        printf("[Linux] Failed to discover PocketBeagle service\n");
        return -1;
    }

    state.address.sin_port = htons(state.port);
    *out_addr = state.address;
    return 0;
}

static void Linux_keyboard_input(JoyPad* ctrl)
{
    const uint8_t* keys = SDL_GetKeyboardState(NULL);
    uint8_t state = 0;

    if(keys[SDL_SCANCODE_RIGHT]) state |= 0x80;
    if(keys[SDL_SCANCODE_LEFT])  state |= 0x40;
    if(keys[SDL_SCANCODE_DOWN])  state |= 0x20;
    if(keys[SDL_SCANCODE_UP])    state |= 0x10;
    if(keys[SDL_SCANCODE_F])     state |= 0x08;
    if(keys[SDL_SCANCODE_G])     state |= 0x04;
    if(keys[SDL_SCANCODE_X])     state |= 0x02;
    if(keys[SDL_SCANCODE_Z])     state |= 0x01;

    if((state & 0xC0) == 0xC0) state &= ~0xC0;
    if((state & 0x30) == 0x30) state &= ~0x30;

    ctrl->state = state;
}

int main(int argc, char** argv) {
    const char* rom_path = NULL;
    if(argc > 1)
        rom_path = argv[1];

    
    struct sockaddr_in server_addr;

    if(Linux_discover(&server_addr) != 0) 
        return 1;

    Graphics gfx;
    JoyPad controller;

    Graphics_init(&gfx);
    Controller_init(&controller);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(connect(sock, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
    }

    printf("[Linux] Connected to PB2 at %s:%u\n",
           inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    char buffer[256];
    ssize_t received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    buffer[received] = '\0';

    printf("[Linux] PB2 banner: %s", buffer);

    if(rom_path) 
    {
        if(Linux_send_rom(sock, rom_path) < 0) 
        {
            printf("[Linux] ROM transfer failed\n");
            close(sock);
        }
    }

    uint16_t udp_port;
    int udp_sock = Linux_create_udp_socket(&udp_port);
    if(udp_sock < 0)
        close(sock);

    uint16_t be_port = htons(udp_port);
    if((Linux_send_frame(sock, PB2_MSG_UDP_PORT, (uint8_t *)&be_port, sizeof(uint16_t)) < 0)
    || Linux_wait_for_frame(sock, PB2_MSG_ACK) <= 0)
    {
        close(udp_sock);
        close(sock);
    }

    Linux_send_frame(sock, PB2_MSG_STREAM_BEGIN, &controller.state, 1);

    bool running = true;
    uint8_t packet[LINUX_PACKET_HEADER_SIZE + LINUX_FRAME_PAYLOAD_SIZE];
    SDL_Event event;
    while(running) {
        while(SDL_PollEvent(&event)) 
        {
            if(event.type == SDL_QUIT)
                running = false;
        }
        Linux_keyboard_input(&controller);
        
        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);
        recvfrom(udp_sock, packet, sizeof(packet), 0, (struct sockaddr*)&from, &from_len);

        uint8_t type = packet[0];
        uint32_t be_len;
        memcpy(&be_len, &packet[1], sizeof(be_len));
        uint32_t length = ntohl(be_len);

        if(type != PB2_MSG_FRAME || length != LINUX_FRAME_PAYLOAD_SIZE)
            continue;

        uint8_t* payload = packet + LINUX_PACKET_HEADER_SIZE;
        uint32_t net_id = 0;
        memcpy(&net_id, payload, sizeof(uint32_t));
        uint32_t frame_id = ntohl(net_id);
        uint8_t* screen = payload + sizeof(uint32_t);
        Graphics_render(&gfx, screen);

        Linux_send_frame(sock, PB2_MSG_INPUT, &controller.state, 1);
    }
    printf("[Linux] Streaming stopped\n");

    close(udp_sock);
    close(sock);
    Graphics_free(&gfx);

    return 0;
}