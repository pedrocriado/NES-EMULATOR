#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Mapper.h"
#include "../Cartridge.h"

static void prg_write(Mapper* mapper, uint16_t addr, uint8_t data);
static void chr_write(Mapper* mapper, uint16_t addr, uint8_t data);
static uint8_t prg_read(Mapper* mapper, uint16_t addr);
static uint8_t chr_read(Mapper* mapper, uint16_t addr);

void set_mapper2(Mapper* mapper, Cartridge* cart)
{
    mapper->prg_write = prg_write;
    mapper->chr_write = chr_write;
    mapper->prg_read = prg_read;
    mapper->chr_read = chr_read;
    mapper->cart = cart;
    mapper->prgClamp = (mapper->prgChunks - 1) * 0x4000;
}

static void prg_write(Mapper* mapper, uint16_t addr, uint8_t data)
{
    if(addr < 0x8000) return;
    uint8_t bank = data % mapper->prgChunks;
    mapper->prgPointerOffset = bank * 0x4000;
}

static void chr_write(Mapper* mapper, uint16_t addr, uint8_t data)
{
    if(mapper->hasChrRam)
        mapper->cart->chrRam[addr & 0x1FFF] = data;
}

static uint8_t prg_read(Mapper* mapper, uint16_t addr)
{
    if(addr < 0x8000) return 0;
    
    if(addr >= 0x8000 && addr < 0xC000)
    {
        return mapper->prgRom[mapper->prgPointerOffset + (addr - 0x8000)];
    }
    else
    {
        return mapper->prgRom[mapper->prgClamp + (addr - 0xC000)];
    }
}

static uint8_t chr_read(Mapper* mapper, uint16_t addr)
{
    if (mapper->hasChrRam) {
        return mapper->cart->chrRam[addr & 0x1FFF];
    } 

    return mapper->cart->chrRom[addr & 0x1FFF];
}
