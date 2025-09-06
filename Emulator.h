#include "CPU6502.h"
#include "Bus.h"

typedef struct Emulator
{
    CPU6502 cpu;
    Bus bus;
} Emulator;

void Emulator_init(Emulator* emu);