#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define RAM_SIZE 0x0800 // 2 KB of RAM
#define PALETTE_RAM 0x20 // 32 bytes
#define PPU_REGISTER 0x0008 // 8 bytes

struct CPU6502;
struct PPU;
struct Cartridge;
struct Controller;

typedef enum IORegister
{ 
    APUPULSE1 = 0x4000,
    // APUPULSE1 = 0x4001,
    // APUPULSE1 = 0x4002,
    // APUPULSE1 = 0x4003,
    // APUPULSE2 = 0x4004,
    // APUPULSE2 = 0x4005,
    // APUPULSE2 = 0x4006,
    // APUPULSE2 = 0x4007,
    // APUPULSE2 = 0x4008,
    // APUPULSE2 = 0x4009,
    // APUPULSE2 = 0x400A,
    // APUPULSE2 = 0x400B,
    // APUPULSE2 = 0x400C,
    // APUPULSE2 = 0x400D,
    // APUPULSE2 = 0x400F,
    // APUPULSE2 = 0x4010,
    // APUPULSE2 = 0x4011,
    // APUPULSE2 = 0x4012,
    // APUPULSE2 = 0x4013,
    OAMDMA    = 0x4014,
    // APUPULSE2 = 0x4015,
    JOY1      = 0x4016,
    JOY2      = 0x4017,
} IORegister;

typedef struct Bus
{
    // devices connected to the bus
    struct CPU6502* cpu;
    struct PPU* ppu;
    struct Cartridge* cart;
    struct JoyPad* controller[2];
    //APU* apu;

    uint8_t dataBus;
    uint8_t ram[RAM_SIZE];
} Bus;

void Bus_init(Bus* bus);
void Bus_free(Bus* bus);
void Bus_write(Bus* bus, uint16_t addr, uint8_t data);
uint8_t Bus_read(Bus* bus, uint16_t addr);