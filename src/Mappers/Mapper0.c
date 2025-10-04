

#include "Mapper.h"

void set_mapper0(Mapper* mapper)
{
    mapper->prg_write = prg_write;
    mapper->chr_write = chr_write;
    mapper->prg_read = prg_read;
    mapper->chr_read = chr_read;

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

void prg_write(Mapper* mapper, uint16_t addr, uint8_t data)
{
    if(addr >= 0x6000 && addr < 0x8000)
    {
        if(mapper->hasPrgRam)
            mapper->cart->prgRam[addr - 0x6000] = data;
    }
}

void chr_write(Mapper* mapper, uint16_t addr, uint8_t data)
{
    if(mapper->hasPrgRam)
        mapper->cart->chrRam[addr & 0x1FFF] = data;
}

uint8_t prg_read(Mapper* mapper, uint16_t addr)
{
    if(addr >= 0x6000 && addr <= 0x7FFF)
    {
        if(mapper->hasPrgRam)
            return mapper->cart->prgRam[addr - 0x6000];
    }
    else if(addr >= 0x8000 && addr <= 0xFFFF)
    {
        uint16_t mapped_addr = addr & (mapper->prgChunks > 1 ? 0x7FFF : 0x3FFF);
        return mapper->cart->prgRom[mapped_addr];
    }

    return 0;
}

uint8_t chr_read(Mapper* mapper, uint16_t addr)
{
    if (mapper->hasChrRam) {
        return mapper->cart->chrRam[addr & 0x1FFF];
    } 

    return mapper->cart->chrRom[addr & 0x1FFF];
}
