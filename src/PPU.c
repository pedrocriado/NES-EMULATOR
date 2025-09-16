#include <stdio.h>

#include "PPU.h"

void PPU_init(PPU* ppu)
{

}

void PPU_write(PPU* ppu, uint16_t addr, uint8_t data)
{
    switch(addr)
    {
        // PPU Registers
        case PPUCTRL:
        case PPUMASK:
        case PPUSTATUS:
        case OAMADDR:
        case OAMDATA:
        case PPUSCROLL:
        case PPUADDR:
        case PPUDATA:
            break;
    }
}

uint8_t PPU_read(PPU* ppu, uint16_t addr)
{
    switch(addr)
    {
        // PPU Registers
        case PPUCTRL:
        case PPUMASK:
        case PPUSTATUS:
        case OAMADDR:
        case OAMDATA:
        case PPUSCROLL:
        case PPUADDR:
        case PPUDATA:
            break;
    }
    return 0;
}