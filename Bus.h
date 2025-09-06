#pragma once

#include <stdio.h>
#include <array>

#include "CPU6502.h"

#define RAM_SIZE 0x0800

typedef struct Bus
{
    // devices connected to the bus
    CPU6502* cpu;

    uint8_t ram[RAM_SIZE];
} Bus;

void Bus_init(Bus* bus);
void Bus_CPU_connect(Bus* bus, CPU6502* cpu);
void Bus_write(Bus* bus, uint16_t addr, uint8_t data);
uint8_t Bus_read(Bus* bus, uint16_t addr);