#include <stdio.h>

#include "PPU.h"

void PPU_init(PPU* ppu)
{
    ppu->screen = malloc(
        sizeof(uint32_t) * VISIBLE_SCANLINES * VISIBLE_DOTS);
}

uint8_t PPU_read(PPU* ppu, uint16_t addr)
{
    addr &= 0x3FFF; // PPU address bus is 14 bits

    if (addr <= 0x1FFF) {
        // Pattern tables (CHR ROM/RAM)
        // TODO: Use cartridge/mapper for CHR ROM/RAM access
        return ppu->pattern_table[addr];
    } else if (addr <= 0x2FFF) {
        // Name tables (mirrored)
        return ppu->name_table[addr & 0x0FFF];
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        // Palette RAM indexes (mirrored every 32 bytes)
        return ppu->palette[addr & 0x1F];
    }
    return 0;
}

void PPU_write(PPU* ppu, uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;

    if (addr <= 0x1FFF) {
        // Pattern tables (CHR RAM only, if present)
        ppu->pattern_table[addr] = data;
    } else if (addr <= 0x2FFF) {
        // Name tables
        ppu->name_table[addr & 0x0FFF] = data;
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        // Palette RAM
        ppu->palette[addr & 0x1F] = data;
    }
}

void PPU_clock(PPU* ppu)
{
    if(ppu->frameComple)
        return;
        
    if(ppu->scanLines < VISIBLE_SCANLINES)
    {
        //TODO: Screen rendering stuff
    }
    else if(ppu->scanLines == VISIBLE_SCANLINES)
    {

    }
    else if(ppu->scanLines < ALL_SCANLINES)
    {
        if(ppu->pixels == 0)
        {
            ppu->status |= STS_VBLANK;
        }
    }
    else
    {

    }

    ppu->pixels++;
    if(ppu->pixels == ALL_DOTS)
    {
        ppu->pixels = 0;
        ppu->scanLines++;
        if(ppu->scanLines == ppu->scanLinePerFame)
        {
            ppu->scanLines = 0;
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
}

void PPU_free(PPU* ppu)
{
    free(ppu->screen);
    free(ppu);
}