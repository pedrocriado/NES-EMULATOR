#pragma once

#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>

#define PIXEL_WIDTH     256 
#define PIXEL_HEIGHT    240
#define PIXEL_SCALE     3
#define GRAPHICS_MENU_CMD_OPEN_ROM 0x1001
#define GRAPHICS_DIALOG_PATH_MAX 1024

// NES palettes (ARGB format)
static const uint32_t nes_palette[64] = {
    0xff666666, 0xff002a88, 0xff1412a7, 0xff3b00a4, 0xff5c007e, 0xff6e0040, 0xff6c0600, 0xff561d00,
    0xff333500, 0xff0b4800, 0xff005200, 0xff004f08, 0xff00404d, 0xff000000, 0xff000000, 0xff000000,
    0xffadadad, 0xff155fd9, 0xff4240ff, 0xff7527fe, 0xffa01acc, 0xffb71e7b, 0xffb53120, 0xff994e00,
    0xff6b6d00, 0xff388700, 0xff0c9300, 0xff008f32, 0xff007c8d, 0xff000000, 0xff000000, 0xff000000,
    0xfffffeff, 0xff64b0ff, 0xff9290ff, 0xffc676ff, 0xfff36aff, 0xfffe6ecc, 0xfffe8170, 0xffea9e22,
    0xffbcbe00, 0xff88d800, 0xff5ce430, 0xff45e082, 0xff48cdde, 0xff4f4f4f, 0xff000000, 0xff000000,
    0xfffffeff, 0xffc0dfff, 0xffd3d2ff, 0xffe8c8ff, 0xfffbc2ff, 0xfffec4ea, 0xfffeccc5, 0xfff7d8a5,
    0xffe4e594, 0xffcfef96, 0xffbdf4ab, 0xffb3f3cc, 0xffb5ebf2, 0xffb8b8b8, 0xff000000, 0xff000000,
};

typedef struct Graphics
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *screenBuffer;
} Graphics;

void Graphics_init(Graphics* grap);
void Graphics_render(Graphics* grap, uint8_t* screen);
void Graphics_free(Graphics* grap);
bool Graphics_prompt_rom_selection(Graphics* grap, char* outPath, size_t outSize);
