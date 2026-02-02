#include "emulator.h"

#define FPS 60
#define FRAME_DELAY (1000 / FPS)

void
init_emulator(Emulator *emulator, char *filename)
{
    init(&emulator->chip, filename);
    init_sdl(&emulator->platform);
}

void
run_emulator(Emulator *emulator)
{
    int quit = 0;

    while(!quit)
    {
        uint32_t frame_start = SDL_GetTicks();
        
        quit = handle_input(&emulator->chip);

        for (int i = 0; i < 10; i++)
            cycle(&emulator->chip);
        
        render_screen(&emulator->platform, &emulator->chip);
        
        uint32_t frame_time = SDL_GetTicks() - frame_start;

        if(FRAME_DELAY > frame_time)
            SDL_Delay(FRAME_DELAY - frame_time);
        
        if(emulator->chip.delay_timer)
            emulator->chip.delay_timer--;
        if(emulator->chip.sound_timer)
            emulator->chip.sound_timer--;
    }

    close_sdl(&emulator->platform);
}