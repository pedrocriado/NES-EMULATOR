#include <stdio.h>
#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <SDL_syswm.h>
#endif

#include "Graphics.h"

static void Graphics_windows_menu(Graphics* grap);
//static void Graphics_mac_menu(Graphics* grap);
//static void Graphics_linux_menu(Graphics* grap);

void Graphics_init(Graphics* grap)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO |
        SDL_INIT_EVENTS) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // Create a window
    grap->window = SDL_CreateWindow(
        "NES Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        PIXEL_WIDTH * PIXEL_SCALE,
        PIXEL_HEIGHT * PIXEL_SCALE,
        SDL_WINDOW_SHOWN
    );
 
    //TODO: Implement menus for Mac(🤮) and Linux(😇)
    // Menu option select
#ifdef _WIN32
    Graphics_windows_menu(grap);
#endif
   

    // Create a renderer
    grap->renderer = SDL_CreateRenderer(
        grap->window, 
        -1, SDL_RENDERER_ACCELERATED
    );

    SDL_RenderSetLogicalSize(
        grap->renderer,
        PIXEL_WIDTH * PIXEL_SCALE,
        PIXEL_HEIGHT * PIXEL_SCALE
    );

    grap->texture = SDL_CreateTexture(
        grap->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        PIXEL_WIDTH,
        PIXEL_HEIGHT
    );
}

void Graphics_render(Graphics* grap, uint32_t* screen)
{
    SDL_UpdateTexture(grap->texture, NULL, screen, PIXEL_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(grap->renderer);
    SDL_RenderCopy(grap->renderer, grap->texture, NULL, NULL);
    SDL_RenderPresent(grap->renderer);
}

void Graphics_free(Graphics* grap)
{
    SDL_DestroyTexture(grap->texture);
    SDL_DestroyRenderer(grap->renderer);
    SDL_DestroyWindow(grap->window);
    SDL_Quit();
}

bool Graphics_prompt_rom_selection(Graphics* grap, char* outPath, size_t outSize)
{
    if(!outPath || outSize == 0)
        return false;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    HWND owner;
    if(SDL_GetWindowWMInfo(grap->window, &wmInfo)) {
        owner = wmInfo.info.win.window;
    }

    char buffer[GRAPHICS_DIALOG_PATH_MAX] = {0};
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = "NES ROMs (*.nes)\0*.nes\0All Files\0*.*\0";
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = GRAPHICS_DIALOG_PATH_MAX;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = "nes";

    if(GetOpenFileNameA(&ofn) == TRUE) {
        strncpy(outPath, buffer, outSize);
        outPath[outSize - 1] = '\0';
        return true;
    }
    return false;
}

static void Graphics_windows_menu(Graphics* grap)
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if(!SDL_GetWindowWMInfo(grap->window, &wmInfo)) {
        SDL_Log("SDL_GetWindowWMInfo failed: %s", SDL_GetError());
        return;
    }

    HWND hwnd = wmInfo.info.win.window;
    if(!hwnd)
        return;

    HMENU menuBar = CreateMenu();
    HMENU fileMenu = CreatePopupMenu();
    if(!menuBar || !fileMenu) {
        SDL_Log("Failed to create Win32 menu.");
        return;
    }

    AppendMenuW(fileMenu, MF_STRING, GRAPHICS_MENU_CMD_OPEN_ROM, L"&Open ROM...");
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)fileMenu, L"&File");

    if(!SetMenu(hwnd, menuBar)) {
        SDL_Log("Failed to set Win32 menu.");
        DestroyMenu(fileMenu);
        DestroyMenu(menuBar);
        return;
    }

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
}