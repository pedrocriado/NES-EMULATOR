#include <stdio.h>

#include "Bus.h"


void Bus_init(Bus* bus)
{
    memset(bus->ram, 0, RAM_SIZE);
}

void Bus_write(Bus* bus, uint16_t addr, uint8_t data)
{

}

uint8_t Bus_read(Bus* bus, uint16_t addr)
{
    return bus->ram[addr];
}