#include <stdio.h>
#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "Graphics.h"

void Graphics_init(Graphics* grap)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // Create a window
    grap->window = SDL_CreateWindow(
        "NES Emulator Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        PIXEL_WIDTH * PIXEL_SCALE,
        PIXEL_HEIGHT * PIXEL_SCALE,
        SDL_WINDOW_SHOWN
    );

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