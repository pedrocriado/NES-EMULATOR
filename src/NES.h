#pragma once

#include <stdbool.h>

#include "CPU6502.h"
#include "PPU.h"
#include "Bus.h"
#include "APU.h"
#include "Cartridge.h"
#include "Graphics.h"
#include "Controller.h"

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
    Bus bus;
    Cartridge cart;
    Graphics graphics;
    JoyPad Controller[2];

    uint8_t tvTiming;
    uint64_t frameTiming;
    bool cartLoaded;
    char currentRomPath[NES_MAX_ROM_PATH];
    char currentRomName[NES_MAX_ROM_NAME];
} NES;

void NES_init(NES* nes, const char *filePath);
void NES_start(NES* nes);
void NES_reset(NES* nes);
void NES_free(NES* nes);
