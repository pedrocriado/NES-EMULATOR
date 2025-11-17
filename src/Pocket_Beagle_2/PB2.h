#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <netinet/in.h>
#include <mdns.h>

#include "../Core/CPU6502.h"
#include "../Core/PPU.h"
#include "../Core/Cartridge.h"
#include "../Core/Controller.h"


#define PB2_TCP_PORT 49733
#define PB2_ROM_BUFFER_SIZE 4096
#define PB2_PACKET_HEADER_SIZE 5
#define PB2_FRAME_PIXELS (256 * 240)
#define PB2_FRAME_PAYLOAD_SIZE (sizeof(uint32_t) + PB2_FRAME_PIXELS)
#define PB2_ROM_UPLOAD_DIR "nes_files/uploads"

#define PB2_SERVICE_TYPE "_haznes._tcp"
#define PB2_SERVICE_NAME "HazardousNES PocketBeagle"

#define PB2_MDNS_DEFAULT_IFACE "usb0"
#define PB2_DNS_SN_NAME "_services._dns-sd._udp.local."

static volatile sig_atomic_t g_keep_running = 1;
static uint8_t g_mdns_send_buffer[1024];

typedef struct PB2Mdns
{
    char hostname[64];
    char hostname_fqdn[96];
    char service_type[64];
    char service_instance[160];
    char txt_board_key[32];
    char txt_board_value[64];
    char txt_role_key[32];
    char txt_role_value[64];

    mdns_string_t hostname_label;
    mdns_string_t hostname_fqdn_label;
    mdns_string_t service_label;
    mdns_string_t service_instance_label;
    mdns_string_t dns_sd_label;

    struct sockaddr_in address_ipv4;
    int port;

    mdns_record_t record_dns_sd;
    mdns_record_t record_ptr;
    mdns_record_t record_srv;
    mdns_record_t record_a;
    mdns_record_t txt_record[2];
} PB2Mdns;

typedef struct PB2Rom
{
    FILE* fp;
    char path[256];
    uint32_t expected_size;
    uint32_t received_size;
} PB2Rom;

typedef struct PB2
{
    CPU6502 cpu;
    PPU ppu;
    Cartridge cart;
    JoyPad controllers[2];
    bool cart_loaded;
} PB2;

typedef struct PB2Session
{
    int fd;
    PB2 emu;
    PB2Rom rom_sink;
    uint32_t next_frame_id;
    struct sockaddr_in client_addr;
    int udp_fd;
    struct sockaddr_in udp_peer;
    bool udp_ready;
} PB2Session;

enum PB2MessageType {
    PB2_MSG_CONTROL = 0x01,
    PB2_MSG_ROM_BEGIN = 0x10,
    PB2_MSG_ROM_CHUNK = 0x11,
    PB2_MSG_ROM_END = 0x12,
    PB2_MSG_STREAM_BEGIN = 0x13,
    PB2_MSG_FRAME = 0x14,
    PB2_MSG_INPUT = 0x15,
    PB2_MSG_UDP_PORT = 0x16,

    PB2_MSG_ACK = 0x80,
    PB2_MSG_ROM_RESULT = 0x81,
    PB2_MSG_ERROR = 0xFF
};

