#include <stdio.h>

#include "PPU.h"

void PPU_init(PPU* ppu)
{
    memset(ppu, 0, sizeof(PPU));

    ppu->screen = malloc(sizeof(uint32_t) * VISIBLE_SCANLINES * VISIBLE_DOTS);
    memset(ppu->screen, 0, sizeof(uint32_t) * VISIBLE_SCANLINES * VISIBLE_DOTS);

    ppu->scanLines = 0;
    ppu->pixels = 0;
    ppu->oddFrame = false;
    ppu->frameComple = false;
    ppu->nmi = false;
}

uint8_t PPU_read(PPU* ppu, uint16_t addr)
{
    addr &= 0x3FFF; // PPU address bus is 14 bits

    if (addr <= 0x1FFF) {
        // TODO: Use cartridge/mapper for CHR ROM/RAM access
        return ppu->pattern_table[addr];
    } else if (addr <= 0x2FFF) {
        return ppu->name_table[addr & 0x0FFF];
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        return ppu->palette[addr & 0x1F];
    }
    return 0;
}

void PPU_write(PPU* ppu, uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;

    if (addr <= 0x1FFF) {
        ppu->pattern_table[addr] = data;
    } else if (addr <= 0x2FFF) {
        ppu->name_table[addr & 0x0FFF] = data;
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        ppu->palette[addr & 0x1F] = data;
    }
}

void PPU_clock(PPU* ppu)
{
    if(ppu->frameComple)
        return;
        
    else if(ppu->scanLines < VISIBLE_SCANLINES)
    { // 0 - 239
        if(ppu->pixels == 0)
        {
            //idle
        }
        else if(ppu->pixels >= 1 && ppu->pixels <= 256)
        {
            // Visible pixel rendering - cycle corresponds to pixel x = cycle-1
            int x = ppu->pixels - 1;
            int y = ppu->scanLines;
            uint8_t palette_index = ((x / 32) + (y / 30)) & 0x3F;
            ppu->screen[y * PPU_SCANLINE + x] = nes_palette_ntsc[palette_index];
            // TODO: proper background + sprite pixel compositing + sprite 0 hit detection
        }
        else if(ppu->pixels >= 257 && ppu->pixels <= 320)
        {
            // TODO: perform sprite evaluation / fetch pattern bytes into sprite shifters

        }
        else if(ppu->pixels >= 321 && ppu->pixels <= 336)
        {
            // TODO: background fetches for next scanline

        }
        else if(ppu->pixels >= 337 && ppu->pixels <= 340)
        {
            // TODO: optional reads (usually ignored)

        }
    }
    else if(ppu->scanLines == VISIBLE_SCANLINES)
    { // 240
        // Idle, nothing happens
    }
    else if(ppu->scanLines < ppu->scanLinesPerFame - 1)
    { // 241 - 260/310
        if(ppu->scanLines == 241 && ppu->pixels == 1)
        {
            ppu->status |= STS_VBLANK;
            if(ppu->ctrl & CTRL_VBLANK)
            {
                ppu->nmi = true;
            }
        }
    }
    else if(ppu->scanLines  < ppu->scanLinesPerFame)
    { // 261
        if(ppu->pixels == 1)
        {
            ppu->status &= ~STS_VBLANK;
            ppu->status &= ~STS_0_HIT;
            ppu->status &= ~STS_OVERFLOW;
            ppu->nmi = false;
        }
        if(ppu->pixels >= 280 && ppu->pixels <= 304)
        {
            if (ppu->mask & 0x18) {
                ppu->v = (uint16_t)((ppu->v & 0x041F) | (ppu->t & 0x7BE0));
            }
        }
        if((ppu->oddFrame) && (ppu->pixels == 339) && (ppu->mask & 0x18)) 
        { // rendering enabled
            ppu->pixels = 0;
            ppu->scanLines = 0;
            ppu->oddFrame = !ppu->oddFrame;
            ppu->frameComple = true;
            return;
        }
    }

    ppu->pixels++;
    if(ppu->pixels == ALL_DOTS)
    {
        ppu->pixels = 0;
        ppu->scanLines++;
        if(ppu->scanLines == ppu->scanLinesPerFame)
        {
            ppu->scanLines = 0;
            ppu->oddFrame = !ppu->oddFrame;
            ppu->frameComple = true;
        }
    }
}

void PPU_set_register(PPU* ppu, uint16_t addr, uint8_t data)
{
    switch(addr)
    {
        case PPUCTRL:
            ppu->ctrl = data;
            ppu->t &= ~NAMETBL_SELECT;
            ppu->t |= (ppu->ctrl & CTRL_NAMETABLE) << 10;
            break;
        case PPUMASK:
            ppu->mask = data;
            break;
        case PPUSTATUS:
            ppu->w = 0;
            break;
        case OAMADDR:
            ppu->oamAddr = data;
            break;
        case OAMDATA:
            ppu->oam[ppu->oamAddr] = data;
            break;
        case PPUSCROLL:
            if(!ppu->w)
            {//first write
                ppu->t &= ~COARSE_X;
                ppu->t |= (data >> 3);
                ppu->x = data & 0x07;
                ppu->w = 1;
            }
            else
            {//second write
                ppu->t &= ~COARSE_Y;
                ppu->t |= (data << 5) & COARSE_Y;
                ppu->t &= ~FINE_Y;
                ppu->t |= (data << 12) & FINE_Y;
                ppu->w = 0;
            }
            break;
        case PPUADDR:
            if(!ppu->w)
            {//first write
                ppu->t = ((data & 0x3F) << 8) | (ppu->t & 0x00FF);
                ppu->w = 1;
            }
            else
            {//second write
                ppu->t = (ppu->t & 0xFF00) | data;
                ppu->v = ppu->t;
                ppu->w = 0;
            }
            break;
        case PPUDATA:
            ppu_wrtie(ppu, ppu->v, data);
            ppu->v += (ppu->ctrl & CTRL_INCREMENT) ? 32 : 1;
            break;
    }
}

uint8_t PPU_get_register(PPU* ppu, uint16_t addr)
{
    uint8_t data = 0x00;

    switch(addr)
    {
        case PPUCTRL:
            break;
        case PPUMASK:
            break;
        case PPUSTATUS:
            data = (ppu->status & 0xE0);
            data |= (ppu->buffer & 0x1F);
            ppu->status &= ~STS_VBLANK;
            ppu->w = 0;
            return ppu->status;
        case OAMADDR:
            break;
        case OAMDATA:
            return ppu->oam[ppu->oamAddr];
        case PPUSCROLL:
            break;
        case PPUADDR:
            break;
        case PPUDATA:
            data = ppu->buffer;
            ppu->buffer = PPU_read(ppu, ppu->v);
            if(ppu->v & 0xC000)
                data = ppu->buffer;
            ppu->v += (ppu->ctrl & CTRL_INCREMENT) ? 32 : 1;
            return data;
    }
    return 0;
}

void PPU_free(PPU* ppu)
{
    free(ppu->screen);
    free(ppu);
}