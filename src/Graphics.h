#include <SDL.h>

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
void Graphics_render(Graphics* grap, uint32_t screen);
void Graphics_free(Graphics* grap);