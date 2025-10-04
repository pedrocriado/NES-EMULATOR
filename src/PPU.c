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
        ppu->cycles = 0;
        ppu->scanLines++;
        if (ppu->scanLines == ALL_SCANLINES)
        {
            ppu->scanLines = 0;
            ppu->frameComple = true;
        }
    }
    else if(ppu->scanLines == VISIBLE_SCANLINES)
    {

    }
    else if(ppu->cycles < ALL_SCANLINES)
    {

    }
    else
    {

    }

    ppu->cycles++;
    if(ppu->cycles == ALL_DOTS)
    {
        ppu->cycles = 0;
        ppu->scanLines++;
        if(ppu->scanLines == ppu->scanLinePerFame)
        {
            ppu->scanLines = 0;
            ppu->frameComple = false;
        }
    }
}

void PPU_set_register(PPU* ppu, uint16_t addr, uint8_t data)
{
    switch(addr)
    {
        case PPUCTRL:
            ppu->ctrl = data;
            break;
        case PPUMASK:
            ppu->mask = data;
            break;
        case PPUSTATUS:
            ppu->status = data;
            break;
        case OAMADDR:
            ppu->oamAddr = data;
            break;
        case OAMDATA:
            ppu->oamData = data;
            break;
        case PPUSCROLL:
            ppu->ppuScroll = data;
            break;
        case PPUADDR:
            ppu->ppuAddr = data;
            break;
        case PPUDATA:
            ppu->ppuData = data;
            break;
    }
}

uint8_t PPU_get_register(PPU* ppu, uint16_t addr)
{
    switch(addr)
    {
        case PPUCTRL:
            return ppu->ctrl;
            break;
        case PPUMASK:
            return ppu->mask;
            break;
        case PPUSTATUS:
            return ppu->status;
            break;
        case OAMADDR:
            return ppu->oamAddr;
            break;
        case OAMDATA:
            return ppu->oamData;
            break;
        case PPUSCROLL:
            return ppu->ppuScroll;
            break;
        case PPUADDR:
            return ppu->ppuAddr;
            break;
        case PPUDATA:
            return ppu->ppuData;
            break;
    }
}

void PPU_free(PPU* ppu)
{
    free(ppu->screen);
}