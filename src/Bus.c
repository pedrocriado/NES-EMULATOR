#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "Bus.h"
#include "CPU6502.h"
#include "PPU.h"
#include "Cartridge.h"
#include "Controller.h"

#include "Mappers/Mapper.h"

void Bus_init(Bus* bus)
{
}

void Bus_write(Bus* bus, uint16_t addr, uint8_t data)
{
    bus->dataBus = data;

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
        switch(addr)
        {
            case OAMDMA:
                uint16_t page = data << 8;
                for(int i = 0; i < 256; i++)
                {
                    uint8_t oam_data = Bus_read(bus, page + i);
                    PPU_set_register(bus->ppu, OAMDATA, oam_data);
                }
                bus->cpu->ic.cycles += 513 + (bus->cpu->ic.cycles & 1);
                break;
            case JOY1:
                Controller_write(bus->controller[0], data);
                break;
        }
    }
    else if(addr <= 0x401F)
    {
        // TODO: Additional APU registers
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
        bus->dataBus = bus->ram[addr % RAM_SIZE];
        return bus->dataBus;
    }
    else if(addr <= 0x3FFF)
    {
        addr = (addr & 0x0007) + 0x2000;
        bus->dataBus = PPU_get_register(bus->ppu, addr);
        return bus->dataBus;
    }
    else if(addr <= 0x4017)
    {
        switch(addr)
        {
            case JOY1:
                bus->dataBus &= 0xE0;
                bus->dataBus |= Controller_read(bus->controller[0]) & 0x01;
                return bus->dataBus;
            default:
                return 0;
        }
    }
    else if(addr <= 0x401F)
    {
        // TODO: Additional APU registers
        return 0;
    }
    else
    {
        bus->dataBus = bus->cart->mapper.prg_read(&bus->cart->mapper, addr);
        return bus->dataBus;
    }
}

void Bus_free(Bus* bus)
{
    free(bus);
}