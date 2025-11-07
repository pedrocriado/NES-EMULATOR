#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "Bus.h"
#include "Cartridge.h"

#define PPU_SCANLINE 256
#define PPU_HEIGHT 240
#define OAM_SIZE 64
#define OAM_CACHE_SIZE 8
#define VISIBLE_SCANLINES 240
#define VISIBLE_DOTS 256
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

typedef enum Bit_Reference
{
    CTRL_NAMETABLE  = 0x03,
    CTRL_INCREMENT  = 0x04,
    CTRL_SP_TABLE   = 0x08,
    CTRL_BG         = 0x10,
    CTRL_SPR_SIZE   = 0x20,
    CTRL_VBLANK     = 0x80,
    MSK_BG_8        = 0x02,
    MSK_SPR_8       = 0x04,
    MSK_BG          = 0x08,
    MSK_SPR         = 0x10,
    MSK_SHOW_ALL    = 0x18,
    STS_VBLANK      = 0x80,
    STS_0_HIT       = 0x40,
    STS_OVERFLOW    = 0x20,
    OAM_PRIORITY    = 0x20,
    OAM_FLIP_X      = 0x40,
    OAM_FLIP_Y      = 0x80,
} Bit_Reference;

// Used by both the t and v internal registers
typedef enum VRAM_Reference
{
    COARSE_X       = 0x001F,
    COARSE_Y       = 0x03E0,
    NAMETBL_X      = 0x0400,
    NAMETBL_Y      = 0x0800,
    FINE_Y         = 0x7000,
    
} VRAM_Reference;

typedef struct OAM
{
    uint8_t y;
    uint8_t idx;
    uint8_t attr;
    uint8_t x;
} OAM;

typedef struct PPU
{
    Bus* bus;
    Cartridge* cart;
    
    // Registers
    uint8_t ctrl;
    uint8_t mask;
    uint8_t status;
    uint8_t oamAddr;
    
    uint8_t dataBus;
    uint8_t ioBus;

    // Internal Registers
    uint16_t v, t;
    uint8_t  x, w;

    uint16_t bgShiftPtrLow;
    uint16_t bgShiftPtrHigh;
    uint16_t bgShiftAttrLow;
    uint16_t bgShiftAttrHigh;

    uint8_t bgNextTileId;
    uint8_t bgNextTileAttr;
    uint8_t bgNextTileLow;
    uint8_t bgNextTileHigh;

    OAM oam[OAM_SIZE];
    
    OAM secondaryOam[OAM_CACHE_SIZE];
    uint8_t secondaryOamIndex[OAM_CACHE_SIZE];
    uint8_t secondaryAddr;

    uint8_t pattern_table[0x2000];
    uint8_t name_table[0x1000];
    uint8_t palette[0x20];

    uint8_t *screen;

    uint16_t scanLinesPerFame;
    uint16_t scanLines;
    uint16_t pixels;
    bool oddFrame;
    bool frameComple;
    bool nmi;
    bool nmiPending;
    uint8_t nmiDelay;
} PPU;

void PPU_init(PPU* ppu);
void PPU_write(PPU* ppu, uint16_t addr, uint8_t data);
uint8_t PPU_read(PPU* ppu, uint16_t addr);
void PPU_clock(PPU* ppu);
void PPU_set_register(PPU* ppu, uint16_t addr, uint8_t data);
uint8_t PPU_get_register(PPU* ppu, uint16_t addr);
void PPU_free(PPU* ppu);