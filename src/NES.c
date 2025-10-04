#include "NES.h"

#include <SDL.h>

void NES_init(NES* nes)
{
    memset(nes, 0, sizeof(NES));
    
    CPU_init(&nes->cpu);
    Bus_init(&nes->bus);
    PPU_init(&nes->ppu);
    Cartridge_init(&nes->cart, &nes->mapper);
    Graphics_init(&nes->graphics);

    Bus_CPU_connect(&nes->bus, &nes->cpu);
    Bus_PPU_connect(&nes->bus, &nes->ppu);
    Bus_Cartridge_connect(&nes->bus, &nes->cart);
}

void NES_start(NES* nes)
{
    //TODO: create proper implementation.
    char *filePath = "DuckTales 2 (USA)";
    Cartridge_load(nes->cart, filePath);

    //North American/Japan and Europe has 
    //different amounts of frames per second.
    switch(nes->mapper->tv)
    {
        case NTSC:
            nes->tvTiming = NTSC_TIMING;
            break;
        case PAL:
            nes->tvTiming = PAL_TIMING;
            break;
    }

    uint64_t freq = SDL_GetPerformanceFrequency();
    nes->fameTiming = (uint64_t)(freq / nes->tvTiming);

    uint64_t prev_time = SDL_GetPerformanceCounter();

    int running = 1;
    SDL_Event event;

    while(true)
    {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT)
                running = 0;
        }

        nes->ppu->frameComple = false;
        while(nes->ppu->frameComple)
        {
            PPU_clock(nes->cpu);
            PPU_clock(nes->cpu);
            PPU_clock(nes->cpu);
            CPU_clock(nes->cpu);
        }     

        Graphics_render(nes->graphics, nes->ppu->screen);

        // Wait until the frame time has elapsed
        uint64_t now = SDL_GetPerformanceCounter();
        uint64_t elapsed = now - prev_time;
        if(elapsed < nes->fameTiming) {
            uint32_t delay_ms = (uint32_t)(((nes->fameTiming - elapsed) * 1000) / freq);
            if(delay_ms > 0) SDL_Delay(delay_ms);
        }
        prev_time = SDL_GetPerformanceCounter();
    }

    NES_reset(nes);
}

void NES_reset(NES* nes)
{
    Mapper_free(nes->mapper);
    Bus_free(nes->bus);
    Cartridge_free(nes->cart);
    CPU_reset(nes->cpu);
    PPU_free(nes->ppu);
    Graphics_free(nes->graphics);
}