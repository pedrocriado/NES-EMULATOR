#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <netinet/in.h>
#include <mdns.h>

#define MAX_ATTEMPTS 25

#define LINUX_PACKET_HEADER_SIZE 5
#define LINUX_ROM_BUFFER_SIZE 4096
#define LINUX_FRAME_PAYLOAD_SIZE (sizeof(uint32_t) + (256 * 240))
#define PB2_SERVICE_TYPE "_haznes._tcp"
#define PB2_SERVICE_NAME "HazardousNES PocketBeagle"

typedef struct DiscoveryState
{
    char instance_name[160];
    char service_type[64];
    char target_host[96];
    struct sockaddr_in address;
    uint16_t port;
    int have_ptr;
    int have_srv;
    int have_a;
} DiscoveryState;

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
