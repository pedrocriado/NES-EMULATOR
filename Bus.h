#pragma once

#include <stdio.h>
#include <stdint.h>

#include "CPU6502.h"

#define RAM_SIZE 0x0800 // 2 Kilobytes of RAM
#define PPU_REGISTERS_SIZE 0x0008 // 8 bytes

typedef struct Bus
{
    // devices connected to the bus
    CPU6502* cpu;
    //PPU* ppu;
    //APU* apu;

    uint8_t ram[RAM_SIZE];
    uint8_t ppu_registers[PPU_REGISTERS_SIZE];
} Bus;

void Bus_init(Bus* bus);
void Bus_write(Bus* bus, uint16_t addr, uint8_t data);
uint8_t Bus_read(Bus* bus, uint16_t addr);