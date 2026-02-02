#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include "chip8.h"

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} Platform;

void init_sdl(Platform *platform);
int handle_input(Chip8 *chip);
void render_screen(Platform *platform, Chip8 *chip);
void close_sdl(Platform *platform);

#endif