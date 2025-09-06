#include "Emulator.h"

void Emulator_init(Emulator* emu)
{
    CPU_init(&emu->cpu);
    Bus_init(&emu->bus);
    Bus_CPU_connect(&emu->bus, &emu->cpu);
}