#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Mapper.h"
#include "../Cartridge.h"

static void prg_write(Mapper* mapper, uint16_t addr, uint8_t data);
static void chr_write(Mapper* mapper, uint16_t addr, uint8_t data);
static uint8_t prg_read(Mapper* mapper, uint16_t addr);
static uint8_t chr_read(Mapper* mapper, uint16_t addr);

void set_mapper0(Mapper* mapper, Cartridge* cart)
{
    mapper->prg_write = prg_write;
    mapper->chr_write = chr_write;
    mapper->prg_read = prg_read;
    mapper->chr_read = chr_read;
    mapper->cart = cart;
    mapper->clamp = (mapper->prgChunks * 0x4000) - 1;
}

static void prg_write(Mapper* mapper, uint16_t addr, uint8_t data)
{
    if(addr >= 0x6000 && addr <= 0x7FFF)
    {
        if(mapper->hasPrgRam)
            mapper->cart->prgRam[addr - 0x6000] = data;
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
        return 0;

    if(addr >= 0x8000 && addr <= 0xFFFF)
    {
        return mapper->prgRom[addr & mapper->clamp];
    }

    return 0;
}

static uint8_t chr_read(Mapper* mapper, uint16_t addr)
{
    if (mapper->hasChrRam) {
        return mapper->cart->chrRam[addr & 0x1FFF];
    } 

    return mapper->chrRom[addr & 0x1FFF];
}
