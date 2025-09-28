#include <stdio.h>

#include "PPU.h"

void PPU_init(PPU* ppu)
{

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

void PPU_write(PPU* ppu, uint16_t addr, uint8_t data)
{
    //TODO
}

uint8_t PPU_read(PPU* ppu, uint16_t addr)
{
    // TODO
    return 0;
}