#include "NES.h"

void NES_init(NES* emu)
{
    CPU_init(emu->cpu);
    Bus_init(emu->bus);
    PPU_init(emu->ppu);
    Bus_CPU_connect(emu->bus, emu->cpu);
    Bus_PPU_connect(emu->bus, emu->ppu);
}

void NES_start(NES* emu)
{

}

void NES_rest(NES* emu)
{
    
}