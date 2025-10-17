#pragma once

#include "CPU6502.h"
#include "PPU.h"
#include "Bus.h"

#include "Bus.h"
#include "CPU6502.h"
#include "PPU.h"
#include "APU.h"
#include "Cartridge.h"
#include "Graphics.h"
#include "Controller.h"

#define NTSC_TIMING 60
#define PAL_TIMING 50
#define NTSC_SCANLINES 262
#define PAL_SCANLINES  312

typedef struct NES
{
    CPU6502 cpu;
    PPU ppu;
    Bus bus;
    Cartridge cart;
    Graphics graphics;
    JoyPad Controller[2];

    uint8_t tvTiming;
    uint64_t fameTiming;
} NES;

void NES_init(NES* nes, char *filePath);
void NES_start(NES* nes);
void NES_reset(NES* nes);
void NES_free(NES* nes);