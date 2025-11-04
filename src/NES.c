#include <SDL.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

//Rom file opening for windows
#ifdef _WIN32
#include <windows.h>
#include <SDL_syswm.h>
#endif

#include "NES.h"
#include "Controller.h"
#include "Graphics.h"

static void NES_update_timing(NES* nes);
static bool NES_load_cartridge(NES* nes, const char* romPath);
static void NES_update_window_title(NES* nes);
static void NES_handle_menu_command(NES* nes, uint32_t command);
static void NES_reset_runtime_state(NES* nes);
static void NES_refresh_controller_state(NES* nes);

void NES_init(NES* nes, const char* filePath)
{ 
    printf("[DEBUG] Initializing NES components\n");
    memset(nes, 0, sizeof(NES));
    

    Bus_init(&nes->bus);
    nes->bus.cpu = &nes->cpu;
    nes->bus.ppu = &nes->ppu;
    nes->bus.cart = &nes->cart;

    Controller_init(&nes->Controller[0]);
    Controller_init(&nes->Controller[1]);
    nes->bus.controller[0] = &nes->Controller[0];
    nes->bus.controller[1] = &nes->Controller[1];

    nes->cpu.bus = &nes->bus;
    nes->ppu.bus = &nes->bus;
    nes->ppu.cart = &nes->cart;

    Graphics_init(&nes->graphics);
    PPU_init(&nes->ppu);

    nes->tvTiming = NTSC_TIMING;
    NES_update_timing(nes);

    if(filePath) {
        if(!NES_load_cartridge(nes, filePath)) {
            printf("[ERROR] Failed to load initial ROM: %s\n", filePath);
        }
    }
}

void NES_start(NES* nes)
{
    int running = 1;
    SDL_Event event;
    uint64_t frequency = SDL_GetPerformanceFrequency();
    uint64_t prevTime = SDL_GetPerformanceCounter();

    while(running) {
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                    if(nes->cartLoaded && event.key.repeat == 0) {
                        NES_refresh_controller_state(nes);
                    }
                    break;
                case SDL_KEYUP:
                    if(nes->cartLoaded) {
                        NES_refresh_controller_state(nes);
                    }
                    break;
#ifdef _WIN32
                case SDL_SYSWMEVENT:
                    if(event.syswm.msg && event.syswm.msg->subsystem == SDL_SYSWM_WINDOWS) {
                        const SDL_SysWMmsg* msg = event.syswm.msg;
                        if(msg->msg.win.msg == WM_COMMAND) {
                            NES_handle_menu_command(nes, LOWORD(msg->msg.win.wParam));
                        }
                    }
                    break;
#endif
            }
        }

        if(nes->cartLoaded) {
            nes->ppu.frameComple = false;
            while(!nes->ppu.frameComple) {
                PPU_clock(&nes->ppu);
                PPU_clock(&nes->ppu);
                PPU_clock(&nes->ppu);
                // Sample the actual PPU NMI line (asserted after the PPU delay),
                // not the internal 'nmiPending' flag which is a scheduling helper.
                nes->cpu.nmiLine = nes->ppu.nmi;
                CPU_clock(&nes->cpu);
                
            }
        }

        Graphics_render(&nes->graphics, nes->ppu.screen);

        uint64_t now = SDL_GetPerformanceCounter();
        uint64_t elapsed = now - prevTime;
        uint64_t target = nes->frameTiming ? nes->frameTiming : (frequency / NTSC_TIMING);
        if(elapsed < target) {
            uint32_t delayMs = (uint32_t)(((target - elapsed) * 1000) / frequency);
            if(delayMs > 0) {
                SDL_Delay(delayMs);
            }
        }
        prevTime = SDL_GetPerformanceCounter();
    }

    NES_free(nes);
}

void NES_reset(NES* nes)
{
    if(nes->cartLoaded && nes->currentRomPath[0] != '\0') {
        NES_load_cartridge(nes, nes->currentRomPath);
    } else {
        NES_reset_runtime_state(nes);
    }
}

void NES_free(NES* nes)
{
    if(!nes) return;
    Cartridge_reset(&nes->cart);
    Controller_init(&nes->Controller[0]);
    Controller_init(&nes->Controller[1]);
    PPU_free(&nes->ppu);
    Graphics_free(&nes->graphics);
    nes->cartLoaded = false;
}

static void NES_reset_runtime_state(NES* nes)
{
    memset(nes->bus.ram, 0, sizeof(nes->bus.ram));
    Controller_init(&nes->Controller[0]);
    Controller_init(&nes->Controller[1]);
}

static void NES_refresh_controller_state(NES* nes)
{
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    uint8_t state = 0;

    if(keys[SDL_SCANCODE_RIGHT]) state |= 0x80;
    if(keys[SDL_SCANCODE_LEFT])  state |= 0x40;
    if(keys[SDL_SCANCODE_DOWN])  state |= 0x20;
    if(keys[SDL_SCANCODE_UP])    state |= 0x10;
    if(keys[SDL_SCANCODE_F])     state |= 0x08;
    if(keys[SDL_SCANCODE_G])     state |= 0x04;
    if(keys[SDL_SCANCODE_X])     state |= 0x02;
    if(keys[SDL_SCANCODE_Z])     state |= 0x01;

    if((state & 0x80) && (state & 0x40)) state &= ~0xC0;
    if((state & 0x20) && (state & 0x10)) state &= ~0x30;

    nes->bus.controller[0]->state = state;
}

static void NES_handle_menu_command(NES* nes, uint32_t command)
{
    switch(command) {
        case GRAPHICS_MENU_CMD_OPEN_ROM: {
            char romPath[NES_MAX_ROM_PATH];
            if(Graphics_prompt_rom_selection(&nes->graphics, romPath, sizeof(romPath)))
                NES_load_cartridge(nes, romPath);
            break;
        }
    }
}

static void NES_update_timing(NES* nes)
{
    switch(nes->cart.mapper.tv) {
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

    uint64_t frequency = SDL_GetPerformanceFrequency();
    nes->frameTiming = frequency / (nes->tvTiming ? nes->tvTiming : NTSC_TIMING);
}

static bool NES_has_nes_extension(const char* name)
{
    size_t len = name ? strlen(name) : 0;
    if(len < 4) return false;
    const char* ext = name + len - 4;
    return (ext[0] == '.') &&
           (ext[1] == 'n') &&
           (ext[2] == 'e') &&
           (ext[3] == 's');
}

static const char* NES_extract_basename(const char* path, char* buffer, size_t bufferSize)
{
    if(!path || !buffer || bufferSize == 0) return NULL;

    const char* lastSlash = strrchr(path, '/');
    const char* lastBack = strrchr(path, '\\');
    const char* base = path;
    if(lastSlash && lastSlash + 1 > base) base = lastSlash + 1;
    if(lastBack && lastBack + 1 > base) base = lastBack + 1;

    size_t copyLen = strlen(base);
    if(NES_has_nes_extension(base)) {
        copyLen -= 4;
    }

    if(copyLen >= bufferSize) copyLen = bufferSize - 1;
    memcpy(buffer, base, copyLen);
    buffer[copyLen] = '\0';
    return buffer;
}

static void NES_update_window_title(NES* nes)
{
    char title[512];
    if(nes->cartLoaded && nes->currentRomName[0] != '\0') {
        snprintf(title, sizeof(title), "NES Emulator - %s", nes->currentRomName);
    } else {
        snprintf(title, sizeof(title), "NES Emulator");
    }
    SDL_SetWindowTitle(nes->graphics.window, title);
}

static bool NES_load_cartridge(NES* nes, const char* romPath)
{
    if(!romPath || romPath[0] == '\0')
        return false;

    printf("[DEBUG] Loading ROM: %s\n", romPath);

    NES_reset_runtime_state(nes);
    
    PPU_free(&nes->ppu);

    Cartridge_load(&nes->cart, romPath);

    nes->bus.cart = &nes->cart;
    nes->ppu.cart = &nes->cart;

    PPU_init(&nes->ppu);
    nes->ppu.bus = &nes->bus;
    nes->ppu.cart = &nes->cart;

    CPU_init(&nes->cpu);

    NES_update_timing(nes);

    strncpy(nes->currentRomPath, romPath, sizeof(nes->currentRomPath));
    nes->currentRomPath[sizeof(nes->currentRomPath) - 1] = '\0';

    NES_extract_basename(nes->currentRomPath, nes->currentRomName, sizeof(nes->currentRomName));
    NES_update_window_title(nes);

    nes->cartLoaded = true;
    printf("[DEBUG] ROM loaded successfully\n");
    return true;
}
