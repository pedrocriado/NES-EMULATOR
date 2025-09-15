#include "CPU6502.h"
#include "PPU.h"
#include "Bus.h"

#include "Bus.h"
#include "CPU6502.h"
#include "PPU.h"
#include "APU.h"

typedef struct NES
{
    CPU6502* cpu;
    PPU* ppu;
    Bus* bus;
} NES;

void NES_init(NES* emu);
void NES_start(NES* emu);
void NES_rest(NES* emu);