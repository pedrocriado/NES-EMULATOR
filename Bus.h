#pragma once
#include <stdio.h>
#include <array>

#include "olc6502.h"

#define RAM_SIZE 64 * 1024

typedef struct
{
    uint8_t ram[RAM_SIZE];
} Bus;

void Bus_init(Bus* bus);
void Bus_write(Bus* bus, uint16_t addr, uint8_t data);
uint8_t Bus_read(Bus* bus, uint16_t addr);