#include <stdio.h>

#include "Bus.h"


void Bus_init(Bus* bus)
{
    memset(bus->ram, 0, RAM_SIZE);
}

void Bus_write(Bus* bus, uint16_t addr, uint8_t data)
{
    if(addr <= 0x1FFF)
    {
        bus->ram[addr % RAM_SIZE] = data;
    }
    else if(addr <= 0x3FFF)
    {
        uint8_t reg = (addr - 0x2000) % PPU_REGISTERS_SIZE;
        bus->ppu_registers[reg] = data;
    }
    else if(addr <= 0x4017)
    {

    }
    else if(addr <= 0x401F)
    {

    }
    else if(addr <= 0xFFFF)
    {

    }

}

uint8_t Bus_read(Bus* bus, uint16_t addr)
{
    if(addr <= 0x1FFF)
    {
        return bus->ram[addr % RAM_SIZE];
    }
    else if(addr <= 0x2007)
    {

    }
    else if(addr <= 0x3FFF)
    {
        
    }
    else if(addr <= 0x4017)
    {

    }
    else if(addr <= 0x401F)
    {

    }
    else if(addr <= 0xFFFF)
    {

    }

    return 0;
}