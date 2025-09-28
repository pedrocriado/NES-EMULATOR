

#include "Mapper.h"

void set_mapper0(Mapper* mapper)
{
    mapper->prg_write = prg_write;
    mapper->chr_write = chr_write;
    mapper->prg_read = prg_read;
    mapper->chr_read = chr_read;
}

void prg_write(Mapper* mapper, uint16_t addr, uint8_t data)
{
    if(addr < 0x4020)
    {
        return;
    }
    else if(addr < 0x6000)
    {
        // The mapper does not use this address
        return;
    }
    else if(addr < 0x8000)
    {

    }
    else if(addr <= 0xFFFF)
    {

    }
}

void chr_write(Mapper* mapper, uint16_t addr, uint8_t data)
{

}

void prg_ram_write(Mapper* mapper, uint16_t addr, uint8_t data)
{

}

void prg_nvram_write(Mapper* mapper, uint16_t addr, uint8_t data)
{

}

void chr_ram_write(Mapper* mapper, uint16_t addr, uint8_t data)
{

}

uint8_t prg_read(Mapper* mapper, uint16_t addr)
{

}

uint8_t chr_read(Mapper* mapper, uint16_t addr)
{

}

uint8_t prg_rom_read(Mapper* mapper, uint16_t addr)
{

}

uint8_t prg_ram_read(Mapper* mapper, uint16_t addr)
{

}

uint8_t prg_nvram_read(Mapper* mapper, uint16_t addr)
{

}

uint8_t chr_rom_read(Mapper* mapper, uint16_t addr)
{

}

uint8_t chr_ram_read(Mapper* mapper, uint16_t addr)
{

}