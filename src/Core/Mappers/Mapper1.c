#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Mapper.h"
#include "../Cartridge.h"

static void prg_write(Mapper* mapper, uint16_t addr, uint8_t data);
static void chr_write(Mapper* mapper, uint16_t addr, uint8_t data);
static uint8_t prg_read(Mapper* mapper, uint16_t addr);
static uint8_t chr_read(Mapper* mapper, uint16_t addr);

void set_mapper1(Mapper* mapper, Cartridge* cart)
{
    mapper->prg_write = prg_write;
    mapper->chr_write = chr_write;
    mapper->prg_read = prg_read;
    mapper->chr_read = chr_read;
    mapper->cart = cart;
    mapper->prgClamp = (mapper->prgChunks - 1) * 0x4000;
    mapper->mmc1.SR = 0x10;
    mapper->mmc1.control = 0x0C;
    mapper->mmc1.prgBank = 0;
    mapper->mmc1.chrBank0 = 0;
    mapper->mmc1.chrBank1 = 0;
}

static void prg_write(Mapper* mapper, uint16_t addr, uint8_t data)
{
    if(addr >= 0x6000 && addr < 0x8000)
    {
        if(mapper->hasPrgRam && (mapper->mmc1.prgBank & 0x10) == 0)
            mapper->cart->prgRam[addr - 0x6000] = data;
        return;
    }

    if(data & 0x80)
    {
        mapper->mmc1.SR = 0x10;
        mapper->mmc1.control |= 0x0C; // Ensure PRG mode 3 after reset
    }    
    else if(mapper->mmc1.SR & 1)
    {
        mapper->mmc1.SR = (mapper->mmc1.SR >> 1) | ((data & 1) << 4);
        if(addr >= 0x8000 && addr <= 0x9FFF)
        {
            mapper->mmc1.control = mapper->mmc1.SR;

            if((mapper->mmc1.control & 0x03) == 0)
            {
                mapper->mirror = ONE_SCREEN_LOWER;
                Mapper_set_mirror(mapper, mapper->mirror);
            }
            else if((mapper->mmc1.control & 0x03) == 1)
            {
                mapper->mirror = ONE_SCREEN_UPPER;
                Mapper_set_mirror(mapper, mapper->mirror);
            }
            else if((mapper->mmc1.control & 0x03) == 2)
            {
                mapper->mirror = VERTICAL;
                Mapper_set_mirror(mapper, mapper->mirror);
            }
            else if((mapper->mmc1.control & 0x03) == 3)
            {
                mapper->mirror = HORIZONTAL;
                Mapper_set_mirror(mapper, mapper->mirror);
            }
        }
        else if(addr >= 0xA000 && addr <= 0xBFFF)
        {
            mapper->mmc1.chrBank0 = mapper->mmc1.SR;
        }
        else if(addr >= 0xC000 && addr <= 0xDFFF)
        {
            mapper->mmc1.chrBank1 = mapper->mmc1.SR;
        }
        else if(addr >= 0xE000 && addr <= 0xFFFF)
        {
            mapper->mmc1.prgBank = mapper->mmc1.SR;   
        }
        mapper->mmc1.SR = 0x10;
    }
    else
    {
        mapper->mmc1.SR = (mapper->mmc1.SR >> 1) | ((data & 1) << 4);
    }
}

static void chr_write(Mapper* mapper, uint16_t addr, uint8_t data)
{
    if(mapper->hasChrRam)
        mapper->cart->chrRam[addr & 0x1FFF] = data;
}

static uint8_t prg_read(Mapper* mapper, uint16_t addr)
{
    if(addr >= 0x6000 && addr <= 0x7FFF)
    {
        if(mapper->hasPrgRam && (mapper->mmc1.prgBank & 0x10) == 0)
            return mapper->cart->prgRam[addr - 0x6000];
        return 0;
    }

    switch((mapper->mmc1.control & 0x0C) >> 2)
    {
        case 0:
        case 1:
            return mapper->prgRom[((mapper->mmc1.prgBank & 0x0E) * 0x4000) + (addr-0x8000)];
        case 2:
            if (addr < 0xC000) 
            {
                return mapper->prgRom[(addr-0x8000)];
            }
            else
            {
                return mapper->prgRom[((mapper->mmc1.prgBank & 0x0F) * 0x4000) + (addr-0xC000)];
            }
        case 3:
            if (addr >= 0xC000) 
            {
                return mapper->prgRom[mapper->prgClamp + (addr-0xC000)];
            }
            else
            {
                return mapper->prgRom[((mapper->mmc1.prgBank & 0x0F) * 0x4000) + (addr-0x8000)];
            }
        default:
            return 0;
    }
}

static uint8_t chr_read(Mapper* mapper, uint16_t addr)
{
    if(mapper->hasChrRam) {
        return mapper->cart->chrRam[addr & 0x1FFF];
    } 

    if((mapper->mmc1.control & 0x10) == 0)
        return mapper->chrRom[((mapper->mmc1.chrBank0 & 0x1E) * 0x1000) + addr];
    else if(addr < 0x1000)
        return mapper->chrRom[(mapper->mmc1.chrBank0 * 0x1000) + addr];
    else
        return mapper->chrRom[(mapper->mmc1.chrBank1 * 0x1000) + addr - 0x1000];
}