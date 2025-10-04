#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "Bus.h"

#define PPU_SCANLINE 256
#define PPU_HEIGHT 240
#define OAM_SIZE 256
#define VISIBLE_SCANLINES 240
#define VISIBLE_DOTS 240
#define ALL_DOTS 341
#define ALL_SCANLINES 262

typedef enum PPU_Registers
{
    PPUCTRL   = 0x2000,
    PPUMASK   = 0x2001,
    PPUSTATUS = 0x2002,
    OAMADDR   = 0x2003,
    OAMDATA   = 0x2004,
    PPUSCROLL = 0x2005,
    PPUADDR   = 0x2006,
    PPUDATA   = 0x2007
} PPU_Registers;

typedef struct PPU
{
    PPUBus* bus;

    // Registers
    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
    uint8_t oamAddr;
    uint8_t oamData;
    uint8_t ppuScroll;
    uint8_t ppuAddr;
    uint8_t ppuData;

    // Internal Registers
    uint16_t v, t;
    uint8_t  x, w;

    uint8_t oat[OAM_SIZE];
    uint8_t pattern_table[0x2000];
    uint8_t name_table[0x1000];
    uint8_t palette[0x20];

    uint32_t *screen;

    uint8_t scanLinePerFame;
    uint8_t scanLines;
    uint16_t cycles;
    bool frameComple;
} PPU;

void PPU_init(PPU* ppu);
void PPU_write(PPU* ppu, uint16_t addr, uint8_t data);
uint8_t PPU_read(PPU* ppu, uint16_t addr);
void PPU_clock(PPU* ppu);
void PPU_set_register(PPU* ppu, uint16_t addr, uint8_t data);
uint8_t PPU_get_register(PPU* ppu, uint16_t addr);
void PPU_free(PPU* ppu);

// NES palettes (ARGB format)
static const uint32_t nes_palette_ntsc[64] = {
    0x545454FF, 0x001E74FF, 0x081090FF, 0x300088FF, 0x440064FF, 0x5C0030FF, 0x540400FF, 0x3C1800FF,
    0x202A00FF, 0x083A00FF, 0x004000FF, 0x003C28FF, 0x002840FF, 0x000000FF, 0x000000FF, 0x000000FF,
    0x989698FF, 0x084CC4FF, 0x3032ECFF, 0x5C1EE4FF, 0x8814B0FF, 0xA01464FF, 0x982220FF, 0x783C00FF,
    0x545A00FF, 0x287200FF, 0x087C00FF, 0x007628FF, 0x006678FF, 0x000000FF, 0x000000FF, 0x000000FF,
    0xECEEECFF, 0x4C9AECFF, 0x787CECFF, 0xB062ECFF, 0xE454ECFF, 0xEC58B4FF, 0xEC6A64FF, 0xD48820FF,
    0xA0AA00FF, 0x74C400FF, 0x4CD020FF, 0x38CC6CFF, 0x38B4CCFF, 0x3C3C3CFF, 0x000000FF, 0x000000FF,
    0xECEEECFF, 0xA8CCECFF, 0xBCBCECFF, 0xD4B2ECFF, 0xECA8ECFF, 0xECA8D4FF, 0xECB4B0FF, 0xE4C490FF,
    0xCCD278FF, 0xB4DE78FF, 0xA8E290FF, 0x98E2B0FF, 0xA0D6E4FF, 0xA0A2A0FF, 0x000000FF, 0x000000FF
};

static const uint32_t nes_palette_pal[64] = {
    0x6A6D6AFF, 0x001E9EFF, 0x1E00D2FF, 0x5A00D2FF, 0x8C00A6FF, 0xA6005AFF, 0xA61E00FF, 0x7C2E00FF,
    0x504600FF, 0x1E5A00FF, 0x006A00FF, 0x005A3CFF, 0x005A7CFF, 0x000000FF, 0x000000FF, 0x000000FF,
    0xB6B6B6FF, 0x1E4EFF, 0x5A32FFFF, 0x8C1EFF, 0xC800C8FF, 0xD21E7CFF, 0xD2461EFF, 0xA66A00FF,
    0x7C8C00FF, 0x32A600FF, 0x00B600FF, 0x00A66AFF, 0x0096B6FF, 0x000000FF, 0x000000FF, 0x000000FF,
    0xFFFFFF, 0x6A96FFFF, 0x8C7CFFFF, 0xC86AFF, 0xFF5AFF, 0xFF5AB6FF, 0xFF7C7CFF, 0xFFB632FF,
    0xD2D200FF, 0xA6E600FF, 0x7CFF32FF, 0x5AFF8CFF, 0x5AFFFF, 0x5A5A5AFF, 0x000000FF, 0x000000FF,
    0xFFFFFF, 0xB6D2FFFF, 0xC8C8FFFF, 0xE6B6FFFF, 0xFFB6FFFF, 0xFFB6E6FF, 0xFFC8C8FF, 0xFFE6B6FF,
    0xFFFFA6FF, 0xE6FFB6FF, 0xC8FFC8FF, 0xB6FFE6FF, 0xB6FFFF, 0xB6B6B6FF, 0x000000FF, 0x000000FF
};