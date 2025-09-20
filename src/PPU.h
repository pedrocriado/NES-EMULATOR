#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "Bus.h"

#define PPU_SCANLINE 256;
#define PPU_HEIGHT 240;

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
    uint8_t v;
    uint8_t t;
    uint8_t x;
    uint8_t w;

    uint8_t scanline;
    uint8_t cycle;
    bool frame_comple;
} PPU;

void PPU_init(PPU* ppu);
void PPU_write(PPU* ppu, uint16_t addr, uint8_t data);
uint8_t PPU_read(PPU* ppu, uint16_t addr);