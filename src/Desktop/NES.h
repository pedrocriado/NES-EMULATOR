#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "Graphics.h"
#include "../Core/CPU6502.h"
#include "../Core/PPU.h"
#include "../Core/APU.h"
#include "../Core/Cartridge.h"
#include "../Core/Controller.h"

#define NTSC_TIMING 60
#define PAL_TIMING 50
#define NTSC_SCANLINES 262
#define PAL_SCANLINES  312
#define NES_MAX_ROM_PATH 1024
#define NES_MAX_ROM_NAME 256

typedef struct NES
{
    CPU6502 cpu;
    PPU ppu;
    Cartridge cart;
    Graphics graphics;
    JoyPad Controller[2];

    uint8_t tvTiming;
    uint64_t frameTiming;
    bool cartLoaded;
    char currentRomPath[NES_MAX_ROM_PATH];
    char currentRomName[NES_MAX_ROM_NAME];
} NES;