#include <stdint.h>

#include "Bus.h"
#include "CPU6502.h"
#include "PPU.h"

#include <string.h>

void Bus_init(Bus* bus)
{
    memset(bus->ram, 0, sizeof(bus->ram));
    bus->cpu = NULL;
}

void Bus_CPU_connect(Bus* bus, CPU6502* cpu)
{
    bus->cpu = cpu;
    cpu->bus = bus;
}

void Bus_PPU_connect(Bus* bus, PPU* ppu)
{
    bus->cpu = ppu;
    ppu->bus = bus;
}

void Bus_write(Bus* bus, uint16_t addr, uint8_t data)
{
    if(addr <= 0x1FFF)
    {
        bus->ram[addr & 0x07FF] = data;
    }
    else if(addr <= 0x3FFF)
    {
        addr = (addr & 0x0007) + 0x2000;
        PPU_wrtie(bus->ppu, addr, data);
    }
    // TODO: Implement 
    else if(addr <= 0x4017)
    {
        bus->ram[addr] = data;
    }
    else if(addr <= 0x401F)
    {
        bus->ram[addr] = data;
    }
    else if(addr <= 0xFFFF)
    {
        bus->ram[addr] = data;
    }
    // TODO: Implement
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
        return PPU_read(bus->ppu, addr);
    }
    // TODO: Implement
    else if(addr <= 0x4017)
    {
        bus->ram[addr];
    }
    else if(addr <= 0x401F)
    {
        bus->ram[addr];
    }
    else if(addr <= 0xFFFF)
    {
        bus->ram[addr];
    }
    // TODO: Implement
    
    return 0;
}