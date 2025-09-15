#include <stdio.h>

#include <SDL.h>

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow(
        "NES Emulator Test",
        0,
        0,
        256 * 2,
        240 * 2,
        SDL_WINDOW_SHOWN
    );

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, 
        -1,
        NULL);

    int running = 1;
    SDL_Event event;

    uint8_t r = 25;
    uint8_t b = 160;
    uint8_t g = 100;
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // Clear screen to blue
        SDL_SetRenderDrawColor(renderer, 0, r, g, b);
        SDL_RenderClear(renderer);

        r++;
        g++;
        b++;

        // Present renderer
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}