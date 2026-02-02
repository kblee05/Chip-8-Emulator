#include "sdl.h"
#include "chip8.h"

#define SCREEN_SCALE 15
#define SCREEN_WIDTH (64 * SCREEN_SCALE)
#define SCREEN_HEIGHT (32 * SCREEN_SCALE)

void
init_sdl(Platform *platform)
{
    platform->window = NULL;
    platform->renderer = NULL;
    platform->texture = NULL;

    platform->window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if(!platform->window)
    {
        perror("Window could not be created");
        return;
    }

    platform->renderer = SDL_CreateRenderer(platform->window, -1, SDL_RENDERER_ACCELERATED);

    if(!platform->renderer)
    {
        perror("Renderer could not be created");
        return;
    }

    platform->texture = SDL_CreateTexture(platform->renderer, SDL_PIXELFORMAT_RGBA8888, 
                                          SDL_TEXTUREACCESS_STREAMING, 64, 32);
}

int
handle_input(Chip8 *chip)
{
    int quit = 0;
    SDL_Event e;

    while(SDL_PollEvent(&e))
    {
        if(e.type == SDL_QUIT)
            quit = 1;
        else if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
        {
            uint8_t state = (e.type == SDL_KEYDOWN) ? 1 : 0;

            switch(e.key.keysym.sym)
            {
                case SDLK_1: chip->keypad[0x1] = state; break;
                case SDLK_2: chip->keypad[0x2] = state; break;
                case SDLK_3: chip->keypad[0x3] = state; break;
                case SDLK_4: chip->keypad[0xC] = state; break;
                case SDLK_q: chip->keypad[0x4] = state; break;
                case SDLK_w: chip->keypad[0x5] = state; break;
                case SDLK_e: chip->keypad[0x6] = state; break;
                case SDLK_r: chip->keypad[0xD] = state; break;
                case SDLK_a: chip->keypad[0x7] = state; break;
                case SDLK_s: chip->keypad[0x8] = state; break;
                case SDLK_d: chip->keypad[0x9] = state; break;
                case SDLK_f: chip->keypad[0xE] = state; break;
                case SDLK_z: chip->keypad[0xA] = state; break;
                case SDLK_x: chip->keypad[0x0] = state; break;
                case SDLK_c: chip->keypad[0xB] = state; break;
                case SDLK_v: chip->keypad[0xF] = state; break;
                default: break;
            }
        }
    }

    return quit;
}

void
render_screen(Platform *platform, Chip8 *chip)
{
    uint32_t pixels[32 * 64];
    
    for (int i = 0; i < 32 * 64; i++) {
        pixels[i] = (chip->display[i]) ? 0xFFFFFFFF : 0x000000FF;
    }
    
    SDL_UpdateTexture(platform->texture, NULL, pixels, 64 * sizeof(uint32_t));
    SDL_RenderCopy(platform->renderer, platform->texture, NULL, NULL);
    SDL_RenderPresent(platform->renderer);
}


void close_sdl(Platform *platform)
{
    SDL_DestroyWindow(platform->window);
    platform->window = NULL;
    SDL_DestroyRenderer(platform->renderer);
    platform->renderer = NULL;
    SDL_DestroyTexture(platform->texture);
    platform->texture = NULL;

    SDL_Quit();
}