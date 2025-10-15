#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "Bus.h"
#include "CPU6502.h"
#include "PPU.h"
#include "Cartridge.h"

#include "Mappers/Mapper.h"

void Bus_init(Bus* bus)
{
}

void Bus_write(Bus* bus, uint16_t addr, uint8_t data)
{
    //printf("writing to %4x\n", addr);
    if(addr <= 0x1FFF)
    {
        bus->ram[addr & 0x07FF] = data;
    }
    else if(addr <= 0x3FFF)
    {
        addr = (addr & 0x0007) + 0x2000;
        PPU_set_register(bus->ppu, addr, data);
    }
    else if(addr <= 0x4017)
    {
        // TODO
    }
    else if(addr <= 0x401F)
    {
        // TODO
    }
    else
    {
        bus->cart->mapper.prg_write(&bus->cart->mapper, addr, data);
    }
}

uint8_t Bus_read(Bus* bus, uint16_t addr)
{
    if(addr <= 0x1FFF)
    {
        return bus->ram[addr % RAM_SIZE];
    }
    else if(addr <= 0x3FFF)
    {
        addr = (addr & 0x0007) + 0x2000;
        return PPU_get_register(bus->ppu, addr);
    }
    else if(addr <= 0x4017)
    {
        // TODO
        return 0;
    }
    else if(addr <= 0x401F)
    {
        // TODO
        return 0;
    }
    else
    {
        return bus->cart->mapper.prg_read(&bus->cart->mapper, addr);
    }
}

void Bus_free(Bus* bus)
{
    free(bus);
}