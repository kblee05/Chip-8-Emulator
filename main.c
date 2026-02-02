#include <stdio.h>
#include "chip8.h"

int main()
{
    Chip8 chip;

    init(&chip);
    cycle(&chip);

    return 0;
}