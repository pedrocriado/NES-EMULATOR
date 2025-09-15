#pragma once

#include <stdio.h>
#include <stdint.h>

#define RAM_SIZE 0x0800 // 2 KB of RAM
#define PPU_REGISTER_SIZE 0x0008 // 8 bytes

struct CPU6502;
struct PPU;

typedef struct Bus
{
    // devices connected to the bus
    struct CPU6502* cpu;
    //PPU* ppu;
    //APU* apu;

    uint8_t ram[RAM_SIZE];
    uint8_t ppu_registers[PPU_REGISTER_SIZE];
} Bus;

void Bus_init(Bus* bus);
void Bus_CPU_connect(Bus* bus, struct CPU6502* cpu);
void Bus_PPU_connect(Bus* bus, struct PPU* ppu);
void Bus_write(Bus* bus, uint16_t addr, uint8_t data);
uint8_t Bus_read(Bus* bus, uint16_t addr);