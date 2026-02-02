#ifndef CHIP8_h
#define CHIP8_h

#include <stdint.h>

typedef struct
{
    uint8_t memory[4096];
    uint8_t v[16];
    uint16_t i;
    uint16_t pc;
    uint8_t sp;
    uint16_t stack[16];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t display[32 * 64];
    uint8_t keypad[16];
} Chip8;

void init(Chip8 *chip, char *file);
void cycle(Chip8 *chip);

#endif