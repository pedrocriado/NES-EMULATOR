#include <stdio.h>

#include "Bus.h"


void Bus_init(Bus* bus)
{
    memset(bus->ram, 0, RAM_SIZE);
}

void Bus_write(Bus* bus, uint16_t addr, uint8_t data)
{
    if(addr >= 0x0000 && addr <= 0xFFFF)
        bus->ram[addr] = data;
}

uint8_t Bus_read(Bus* bus, uint16_t addr)
{
    if(addr >= 0x0000 && addr <= 0xFFFF)
        return bus->ram[addr];
        
    return 0x00;
}