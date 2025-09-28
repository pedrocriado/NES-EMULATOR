#include "NES.h"

void NES_init(NES* nes)
{
    memset(nes, 0, sizeof(NES));
    
    CPU_init(nes->cpu);
    Bus_init(nes->bus);
    PPU_init(nes->ppu);
    Cartridge_init(nes->cart);

    Bus_CPU_connect(nes->bus, nes->cpu);
    Bus_PPU_connect(nes->bus, nes->ppu);
    Bus_Cartridge_connect(nes->bus, nes->cart);
}

void NES_start(NES* emu)
{

}

void NES_rest(NES* emu)
{
    
}