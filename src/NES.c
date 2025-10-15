#include "NES.h"

#include <SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void NES_init(NES* nes, char *filePath)
{
    memset(nes, 0, sizeof(NES));
        
    printf("[DEBUG] About to load cartridge\n");
    Cartridge_load(&nes->cart, filePath);
    printf("[DEBUG] Cartridge loaded, Mapper ID: %d\n", nes->cart.mapperId);

    printf("[DEBUG] About to load NES conponents.\n");
    Bus_init(&nes->bus);
    nes->bus.cpu = &nes->cpu;
    nes->bus.ppu = &nes->ppu;
    nes->bus.cart = &nes->cart;
    nes->ppu.bus = &nes->bus;
    nes->ppu.cart = &nes->cart;
    nes->cpu.bus = &nes->bus;
    PPU_init(&nes->ppu);
    CPU_init(&nes->cpu);
    Graphics_init(&nes->graphics);
    printf("[DEBUG] NES conponents loaded.\n");
}

void NES_start(NES* nes)
{
    if (!nes) {
        printf("[ERROR] NES struct is NULL\n");
        exit(EXIT_FAILURE);
    }
    
    //North American/Japan and Europe has 
    //different amounts of frames per second.
    switch(nes->cart.mapper.tv)
    {
        case NTSC:
            nes->tvTiming = NTSC_TIMING;
            nes->ppu.scanLinesPerFame = NTSC_SCANLINES;
            break;
        case PAL:
            nes->tvTiming = PAL_TIMING;
            nes->ppu.scanLinesPerFame = PAL_SCANLINES;
            break;
        default:
            nes->tvTiming = NTSC_TIMING;
            nes->ppu.scanLinesPerFame = NTSC_SCANLINES;
            break;
    }

    uint64_t freq = SDL_GetPerformanceFrequency();
    nes->fameTiming = (uint64_t)(freq / nes->tvTiming);

    uint64_t prev_time = SDL_GetPerformanceCounter();

    int running = 1;
    SDL_Event event;

    while(running)
    {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT)
                running = 0;
        }
    
        nes->ppu.frameComple = false;
        while(!nes->ppu.frameComple)
        {
            PPU_clock(&nes->ppu);
            PPU_clock(&nes->ppu);
            PPU_clock(&nes->ppu);
            CPU_clock(&nes->cpu);
            if(nes->ppu.nmi)
            {
                nes->ppu.nmi = false;
                CPU_nmi(&nes->cpu);
            }
        }     
        
        Graphics_render(&nes->graphics, nes->ppu.screen);

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
    SDL_Quit();
}

void NES_reset(NES* nes)
{
    Mapper_free(&nes->cart.mapper);
    Bus_free(&nes->bus);
    Cartridge_free(&nes->cart);
    CPU_reset(&nes->cpu);
    PPU_free(&nes->ppu);
    Graphics_free(&nes->graphics);
}

void NES_free(NES* nes)
{
    Bus_free(&nes->bus);
    Cartridge_free(&nes->cart);
    CPU_free(&nes->cpu);
    PPU_free(&nes->ppu);
    Graphics_free(&nes->graphics);
}