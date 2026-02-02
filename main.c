#include <stdio.h>
#include <unistd.h>
#include "emulator.h"

int main(int argc, char **argv)
{
    if(argc != 2)
        return 1;

    Emulator emulator;
    
    init_emulator(&emulator, argv[1]);

    run_emulator(&emulator);
    
    return 0;
}