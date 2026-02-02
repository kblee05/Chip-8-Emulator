#include "sdl.h"
#include <SDL2/SDL.h>
#include "chip8.h"

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

SDL_Window *window = NULL;
SDL_Surface *screen_surface = NULL;

void
init_sdl(Chip8 *chip)
{

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        perror("SDL could not initialize");
        return;
    }

    window = SDL_CreateWindow("Chip-8 Emulator",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);

    if(!window)
    {
        perror("Window could not be created");
        return;
    }

    screen_surface = SDL_GetWindowSurface(window);
    SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);
    
    SDL_Event e;
    int quit = 0;

    while(!quit)
    {
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                quit = 1;
            
            if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
            {
                int is_pressed = (e.type == SDL_KEYDOWN);

                switch(e.key.keysym.sym)
                {
                    case SDLK_1: chip->keypad[0x0] = is_pressed; break;
                    case SDLK_2: chip->keypad[0x1] = is_pressed; break;
                    case SDLK_3: chip->keypad[0x2] = is_pressed; break;
                    case SDLK_4: chip->keypad[0x3] = is_pressed; break;
                    case SDLK_q: chip->keypad[0x4] = is_pressed; break;
                    case SDLK_w: chip->keypad[0x5] = is_pressed; break;
                    case SDLK_e: chip->keypad[0x6] = is_pressed; break;
                    case SDLK_r: chip->keypad[0x7] = is_pressed; break;
                    case SDLK_a: chip->keypad[0x8] = is_pressed; break;
                    case SDLK_s: chip->keypad[0x9] = is_pressed; break;
                    case SDLK_d: chip->keypad[0xA] = is_pressed; break;
                    case SDLK_f: chip->keypad[0xB] = is_pressed; break;
                    case SDLK_z: chip->keypad[0xC] = is_pressed; break;
                    case SDLK_x: chip->keypad[0xD] = is_pressed; break;
                    case SDLK_c: chip->keypad[0xE] = is_pressed; break;
                    case SDLK_v: chip->keypad[0xF] = is_pressed; break;
                }
            }
        }
    }
}

void
exit_sdl()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}