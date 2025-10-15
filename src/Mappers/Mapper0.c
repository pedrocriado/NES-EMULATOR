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

    switch (mapper->mirror) {
        case HORIZONTAL:
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2000;
            mapper->name_table_map[2] = 0x2400;
            mapper->name_table_map[3] = 0x2400;
            break;
        case VERTICAL:
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2400;
            mapper->name_table_map[2] = 0x2000;
            mapper->name_table_map[3] = 0x2400;
            break;
        case ONE_SCREEN_LOWER:
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2000;
            mapper->name_table_map[2] = 0x2000;
            mapper->name_table_map[3] = 0x2000;
            break;
        case ONE_SCREEN_UPPER:
            mapper->name_table_map[0] = 0x2400;
            mapper->name_table_map[1] = 0x2400;
            mapper->name_table_map[2] = 0x2400;
            mapper->name_table_map[3] = 0x2400;
            break;
        case FOUR_SCREEN:
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2400;
            mapper->name_table_map[2] = 0x2800;
            mapper->name_table_map[3] = 0x2C00;
            break;
        default:
            // Default to horizontal if unsupported
            mapper->name_table_map[0] = 0x2000;
            mapper->name_table_map[1] = 0x2000;
            mapper->name_table_map[2] = 0x2400;
            mapper->name_table_map[3] = 0x2400;
            break;
    }
}

static void prg_write(Mapper* mapper, uint16_t addr, uint8_t data)
{
    if(addr >= 0x6000 && addr < 0x8000)
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
    {
        if(mapper->hasPrgRam)
            return mapper->cart->prgRam[addr - 0x6000];
        return 0;
    }
    else if(addr >= 0x8000 && addr <= 0xFFFF)
    {
        // For 16KB PRG ROM, mirror the bank
        if (mapper->cart->prgChunks == 1) {
            addr = (addr & 0x3FFF); // Mirror 0x8000-0xBFFF to 0xC000-0xFFFF
        } else {
            addr = (addr & 0x7FFF); // Use full 32KB
        }
        return mapper->cart->prgRom[addr];
    }

    return 0;
}

static uint8_t chr_read(Mapper* mapper, uint16_t addr)
{
    if (mapper->hasChrRam) {
        return mapper->cart->chrRam[addr & 0x1FFF];
    } 

    return mapper->cart->chrRom[addr & 0x1FFF];
}
