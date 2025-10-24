#pragma once

#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>

#define PIXEL_WIDTH     256 
#define PIXEL_HEIGHT    240
#define PIXEL_SCALE     3
#define GRAPHICS_MENU_CMD_OPEN_ROM 0x1001
#define GRAPHICS_DIALOG_PATH_MAX 1024

typedef struct Graphics
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} Graphics;

void Graphics_init(Graphics* grap);
void Graphics_render(Graphics* grap, uint32_t* screen);
void Graphics_free(Graphics* grap);
bool Graphics_prompt_rom_selection(Graphics* grap, char* outPath, size_t outSize);
