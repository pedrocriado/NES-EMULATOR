#include "CPU6502.h"
#include "PPU.h"
#include "Bus.h"

#include "Bus.h"
#include "CPU6502.h"
#include "PPU.h"
#include "APU.h"
#include "Cartridge.h"
#include "Mapper.h"
#include "Graphics.h"

#define NTSC_TIMING 60
#define PAL_TIMING 50

typedef struct NES
{
    CPU6502* cpu;
    PPU* ppu;
    Bus* bus;
    Cartridge* cart;
    Mapper* mapper;
    Graphics* graphics;

    uint8_t tvTiming;
    uint64_t fameTiming;
} NES;

void NES_init(NES* emu);
void NES_start(NES* emu);
void NES_reset(NES* emu);