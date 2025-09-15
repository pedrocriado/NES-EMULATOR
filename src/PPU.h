#pragma once

#include <stdio.h>
#include <stdint.h>

#include "Bus.h"

#define PPU_SCANLINE 256;
#define PPU_HEIGHT 240;

typedef struct PPU
{
    Bus* bus;
} PPU;

void PPU_init(PPU* ppu);