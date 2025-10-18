#include "NES.h"

#include <SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static uint8_t NES_input_key(SDL_Scancode key);

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
    Controller_init(&nes->Controller[0]);
    Controller_init(&nes->Controller[1]);
    nes->bus.controller[0] = &nes->Controller[0];
    nes->bus.controller[1] = &nes->Controller[1];

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
    uint8_t key;
    static int frame_count = 0;

    while(running)
    {
        while(SDL_PollEvent(&event)) {
            switch(event.type)
            {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                    key = NES_input_key(event.key.keysym.scancode);
                    printf("down: %x\n", key);
                    nes->bus.controller[0]->status |= key;
                    printf("states: %x\n", nes->bus.controller[0]->status);
                    break;
                case SDL_KEYUP:
                    key = NES_input_key(event.key.keysym.scancode);
                    printf("up: %x\n", key);
                    nes->bus.controller[0]->status &= ~key;
                    printf("states: %x\n", nes->bus.controller[0]->status);
                    break;
            }
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

static uint8_t NES_input_key(SDL_Scancode key)
{
    switch(key)
    {
        case SDL_SCANCODE_RIGHT: return 0x80;
        case SDL_SCANCODE_LEFT:  return 0x40;
        case SDL_SCANCODE_DOWN:  return 0x20;
        case SDL_SCANCODE_UP:    return 0x10;
        case SDL_SCANCODE_F:     return 0x08;
        case SDL_SCANCODE_G:     return 0x04;
        case SDL_SCANCODE_X:     return 0x02;
        case SDL_SCANCODE_Z:     return 0x01;
        default:                 return 0x00;
    }
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