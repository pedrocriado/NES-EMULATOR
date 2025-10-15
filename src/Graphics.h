#pragma once

#include <SDL.h>
#include <stdlib.h>

#define PIXEL_WIDTH     256 
#define PIXEL_HEIGHT    240
#define PIXEL_SCALE     2

typedef struct Graphics
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} Graphics;

void Graphics_init(Graphics* grap);
void Graphics_render(Graphics* grap, uint32_t* screen); // <-- take pointer
void Graphics_free(Graphics* grap);