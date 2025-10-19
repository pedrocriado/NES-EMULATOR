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
    uint8_t secondaryAddr;

    //sprite evalution variables
    uint8_t primaryCursor, secondaryCursor;
    uint8_t sprM, sprN, sprTemp;
    OAM tmpOAM;
    uint8_t sprDone, sprHeight, sprCopying;

    uint8_t pattern_table[0x2000];
    uint8_t name_table[0x1000];
    uint8_t palette[0x20];

    uint32_t *screen;

    uint16_t scanLinesPerFame;
    uint16_t scanLines;
    uint16_t pixels;
    bool oddFrame;
    bool frameComple;
    bool nmi;
} PPU;

void PPU_init(PPU* ppu);
void PPU_write(PPU* ppu, uint16_t addr, uint8_t data);
uint8_t PPU_read(PPU* ppu, uint16_t addr);
void PPU_clock(PPU* ppu);
void PPU_set_register(PPU* ppu, uint16_t addr, uint8_t data);
uint8_t PPU_get_register(PPU* ppu, uint16_t addr);
void PPU_free(PPU* ppu);

// NES palettes (ARGB format)
static const uint32_t nes_palette[64] = {
    0xff666666, 0xff002a88, 0xff1412a7, 0xff3b00a4, 0xff5c007e, 0xff6e0040, 0xff6c0600, 0xff561d00,
    0xff333500, 0xff0b4800, 0xff005200, 0xff004f08, 0xff00404d, 0xff000000, 0xff000000, 0xff000000,
    0xffadadad, 0xff155fd9, 0xff4240ff, 0xff7527fe, 0xffa01acc, 0xffb71e7b, 0xffb53120, 0xff994e00,
    0xff6b6d00, 0xff388700, 0xff0c9300, 0xff008f32, 0xff007c8d, 0xff000000, 0xff000000, 0xff000000,
    0xfffffeff, 0xff64b0ff, 0xff9290ff, 0xffc676ff, 0xfff36aff, 0xfffe6ecc, 0xfffe8170, 0xffea9e22,
    0xffbcbe00, 0xff88d800, 0xff5ce430, 0xff45e082, 0xff48cdde, 0xff4f4f4f, 0xff000000, 0xff000000,
    0xfffffeff, 0xffc0dfff, 0xffd3d2ff, 0xffe8c8ff, 0xfffbc2ff, 0xfffec4ea, 0xfffeccc5, 0xfff7d8a5,
    0xffe4e594, 0xffcfef96, 0xffbdf4ab, 0xffb3f3cc, 0xffb5ebf2, 0xffb8b8b8, 0xff000000, 0xff000000,
};