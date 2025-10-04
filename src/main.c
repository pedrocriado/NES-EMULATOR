#include <stdio.h>
#include <SDL.h>

#define PIXEL_WIDTH     256 
#define PIXEL_HEIGHT    240
#define PIXEL_SCALE     2

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // Create a window
    SDL_Window *window = SDL_CreateWindow(
        "NES Emulator Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        PIXEL_WIDTH * PIXEL_SCALE,
        PIXEL_HEIGHT * PIXEL_SCALE,
        SDL_WINDOW_SHOWN
    );

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, 
        -1, SDL_RENDERER_ACCELERATED
    );

    SDL_RenderSetLogicalSize(
        renderer,
        PIXEL_WIDTH * PIXEL_SCALE,
        PIXEL_HEIGHT * PIXEL_SCALE
    );

    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        PIXEL_WIDTH,
        PIXEL_HEIGHT
    );

    // Allocate NES-sized pixel buffer
    uint32_t *buffer = (uint32_t*)malloc(PIXEL_WIDTH * PIXEL_HEIGHT * sizeof(uint32_t));
    if (!buffer) {
        printf("Failed to allocate buffer\n");
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Fill buffer with a color pattern
    for (int y = 0; y < PIXEL_HEIGHT; y++) {
        for (int x = 0; x < PIXEL_WIDTH; x++) {
            
            buffer[y * PIXEL_WIDTH + x] = 0xFF000000;
        }
    }

    // Update texture with buffer
    SDL_UpdateTexture(texture, NULL, buffer, PIXEL_WIDTH * sizeof(uint32_t));

    // Render loop
    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    free(buffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}