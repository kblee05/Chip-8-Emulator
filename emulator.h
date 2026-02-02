#ifndef EMULATOR_H
#define EMULATOR_H

#include "chip8.h"
#include "sdl.h"

typedef struct
{
    Chip8 chip;
    Platform platform;
} Emulator;

void init_emulator(Emulator *emulator, char *filename);
void run_emulator(Emulator *emulator);

#endif