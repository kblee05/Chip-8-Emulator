#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "sdl.h"
#include "chip8.h"

#define FONTSET_START_ADDRESS 0x50

const uint8_t chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

#define START_LOCATION 0x200

static void load_rom(Chip8 *chip, char *file);

void
init(Chip8 *chip, char *file)
{
    load_rom(chip, file);
    chip->pc = START_LOCATION;
    chip->sp = 0;
    chip->i = 0;
    chip->delay_timer = 0;
    chip->sound_timer = 0;
    
    for(int i = 0; i < 16; i++)
    {
        chip->v[i] = 0;
        chip->stack[i] = 0;
        chip->keypad[i] = 0;
    }

    for(int i = 0; i < 32 * 64; i++)
            chip->display[i] = 0;
    
    memcpy(&chip->memory[FONTSET_START_ADDRESS], chip8_fontset, sizeof(chip8_fontset));

    srand(time(NULL));
}

static void
load_rom(Chip8 *chip, char *file)
{
    FILE *input = fopen(file, "rb");

    if(!file)
    {
        perror("could not open rom.txt");
        return;
    }

    fseek(input, 0, SEEK_END);
    int size = ftell(input);
    fseek(input, 0, SEEK_SET);

    fread(&chip->memory[0x200], 1, size, input);

    fclose(input);
}

typedef void (*Chip8Handler)(Chip8 *, uint16_t);

static void op_0nnn(Chip8 *chip, uint16_t opcode);
static void op_1nnn(Chip8 *chip, uint16_t opcode);
static void op_2nnn(Chip8 *chip, uint16_t opcode);
static void op_3nnn(Chip8 *chip, uint16_t opcode);
static void op_4nnn(Chip8 *chip, uint16_t opcode);
static void op_5nnn(Chip8 *chip, uint16_t opcode);
static void op_6nnn(Chip8 *chip, uint16_t opcode);
static void op_7nnn(Chip8 *chip, uint16_t opcode);
static void op_8nnn(Chip8 *chip, uint16_t opcode);
static void op_9nnn(Chip8 *chip, uint16_t opcode);
static void op_Annn(Chip8 *chip, uint16_t opcode);
static void op_Bnnn(Chip8 *chip, uint16_t opcode);
static void op_Cnnn(Chip8 *chip, uint16_t opcode);
static void op_Dnnn(Chip8 *chip, uint16_t opcode);
static void op_Ennn(Chip8 *chip, uint16_t opcode);
static void op_Fnnn(Chip8 *chip, uint16_t opcode);

Chip8Handler main_table[16] = {
    &op_0nnn,
    &op_1nnn,
    &op_2nnn,
    &op_3nnn,
    &op_4nnn,
    &op_5nnn,
    &op_6nnn,
    &op_7nnn,
    &op_8nnn,
    &op_9nnn,
    &op_Annn,
    &op_Bnnn,
    &op_Cnnn,
    &op_Dnnn,
    &op_Ennn,
    &op_Fnnn,
};

void
cycle(Chip8 *chip)
{
    uint16_t opcode = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc + 1];
    chip->pc += 2;

    uint8_t msb4 = (opcode & 0xF000) >> 12;

    main_table[msb4](chip, opcode);
}

static void op_00E0(Chip8 *chip);
static void op_00EE(Chip8 *chip);

static void
op_0nnn(Chip8 *chip, uint16_t opcode)
{
    uint16_t byte = opcode & 0x00FF;

    switch(byte)
    {
        case 0xE0:
            op_00E0(chip);
            break;
        
        case 0xEE:
            op_00EE(chip);
            break;
        
        default:
            perror("Unkown 0-series opcode");
            break;
    }
}

/*
    00E0 - CLS
    Clear the display
*/

static void
op_00E0(Chip8 *chip)
{
    memset(chip->display, 0, sizeof(chip->display));
}

/*
    00EE - RET
    Return from a subroutine.

    The interpreter sets the program counter to the address 
    at the top of the stack, then subtracts 1 from the stack pointer.
*/

static void
op_00EE(Chip8 *chip)
{
    chip->pc = chip->stack[--chip->sp];
}

/*
    1nnn - JP addr
    Jump to location nnn.

    The interpreter sets the program counter to nnn.
*/

static void
op_1nnn(Chip8 *chip, uint16_t opcode)
{
    uint16_t addr = opcode & 0x0FFF;

    chip->pc = addr;
}

/*
    2nnn - CALL addr
    Call subroutine at nnn.

    The interpreter increments the stack pointer, then puts the 
    current PC on the top of the stack. The PC is then set to nnn.
*/

static void
op_2nnn(Chip8 *chip, uint16_t opcode)
{
    uint16_t addr = opcode & 0x0FFF;

    chip->stack[chip->sp++] = chip->pc;
    chip->pc = addr;
}

/*
    3xkk - SE Vx, byte
    Skip next instruction if Vx = kk.

    The interpreter compares register Vx to kk, and if 
    they are equal, increments the program counter by 2.
*/

static void
op_3nnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    if(chip->v[x] == kk)
        chip->pc += 2;
}

/*
    4xkk - SNE Vx, byte
    Skip next instruction if Vx != kk.

    The interpreter compares register Vx to kk, and if 
    they are not equal, increments the program counter by 2.
*/

static void
op_4nnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    if(chip->v[x] != kk)
        chip->pc += 2;
}

/*
    5xy0 - SE Vx, Vy
    Skip next instruction if Vx = Vy.

    The interpreter compares register Vx to register Vy, 
    and if they are equal, increments the program counter by 2.
*/

static void
op_5nnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if(chip->v[x] == chip->v[y])
        chip->pc += 2;
}

/*
6xkk - LD Vx, byte
Set Vx = kk.

The interpreter puts the value kk into register Vx.
*/

static void
op_6nnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    chip->v[x] = kk;
}

/*
    7xkk - ADD Vx, byte
    Set Vx = Vx + kk.

    Adds the value kk to the value of register Vx, 
    then stores the result in Vx.
*/

static void
op_7nnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    chip->v[x] += kk;
}

static void op_8xy0(Chip8 *chip, uint16_t opcode);
static void op_8xy1(Chip8 *chip, uint16_t opcode);
static void op_8xy2(Chip8 *chip, uint16_t opcode);
static void op_8xy3(Chip8 *chip, uint16_t opcode);
static void op_8xy4(Chip8 *chip, uint16_t opcode);
static void op_8xy5(Chip8 *chip, uint16_t opcode);
static void op_8xy6(Chip8 *chip, uint16_t opcode);
static void op_8xy7(Chip8 *chip, uint16_t opcode);
static void op_8xyE(Chip8 *chip, uint16_t opcode);

Chip8Handler op8nnn_table[16] = {
    &op_8xy0,
    &op_8xy1,
    &op_8xy2,
    &op_8xy3,
    &op_8xy4,
    &op_8xy5,
    &op_8xy6,
    &op_8xy7,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &op_8xyE,
    NULL
};

static void
op_8nnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t n = opcode & 0x000F;

    if(op8nnn_table[n])
        op8nnn_table[n](chip, opcode);
    else
        perror("Unkown 8-series opcode");
}

/*
    8xy0 - LD Vx, Vy
    Set Vx = Vy.

    Stores the value of register Vy in register Vx.
*/

static void
op_8xy0(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    chip->v[x] = chip->v[y];
}

/*
    8xy1 - OR Vx, Vy
    Set Vx = Vx OR Vy.

    Performs a bitwise OR on the values of Vx and Vy, 
    then stores the result in Vx. A bitwise OR compares the 
    corrseponding bits from two values, and if either bit is 1, 
    then the same bit in the result is also 1. Otherwise, it is 0.
*/

static void
op_8xy1(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    chip->v[x] = chip->v[x] | chip->v[y];
}

/*
    8xy2 - AND Vx, Vy
    Set Vx = Vx AND Vy.

    Performs a bitwise AND on the values of Vx and Vy, 
    then stores the result in Vx. A bitwise AND compares the 
    corrseponding bits from two values, and if both bits are 1, 
    then the same bit in the result is also 1. Otherwise, it is 0.
*/

static void
op_8xy2(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    chip->v[x] = chip->v[x] & chip->v[y];
}

/*
    8xy3 - XOR Vx, Vy
    Set Vx = Vx XOR Vy.

    Performs a bitwise exclusive OR on the values of Vx and Vy, 
    then stores the result in Vx. An exclusive OR compares the 
    corrseponding bits from two values, and if the bits are not 
    both the same, then the corresponding bit in the result is 
    set to 1. Otherwise, it is 0.
*/

static void
op_8xy3(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    chip->v[x] = chip->v[x] ^ chip->v[y];
}

/*
    8xy4 - ADD Vx, Vy
    Set Vx = Vx + Vy, set VF = carry.

    The values of Vx and Vy are added together. If the result is 
    greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. 
    Only the lowest 8 bits of the result are kept, and stored in Vx.
*/

static void
op_8xy4(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    uint16_t sum = chip->v[x] + chip->v[y];
    chip->v[0xF] = (sum > 0xFF) ? 1 : 0;
    chip->v[x] = sum & 0xFF;
}

/*
    8xy5 - SUB Vx, Vy
    Set Vx = Vx - Vy, set VF = NOT borrow.

    If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy 
    is subtracted from Vx, and the results stored in Vx.
*/

static void
op_8xy5(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    chip->v[0xF] = (chip->v[x] > chip->v[y]) ? 1 : 0;
    
    chip->v[x] -= chip->v[y];
}

/*
    8xy6 - SHR Vx {, Vy}
    Set Vx = Vx SHR 1.

    If the least-significant bit of Vx is 1, then VF is set 
    to 1, otherwise 0. Then Vx is divided by 2.
*/

static void
op_8xy6(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;

    chip->v[0xF] = chip->v[x] & 1;

    chip->v[x] >>= 1;
}

/*
    8xy7 - SUBN Vx, Vy
    Set Vx = Vy - Vx, set VF = NOT borrow.

    If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is 
    subtracted from Vy, and the results stored in Vx.
*/

static void
op_8xy7(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    chip->v[0xF] = (chip->v[y] > chip->v[x]) ? 1 : 0;

    chip->v[x] = chip->v[y] - chip->v[x];
}

/*
    8xyE - SHL Vx {, Vy}
    Set Vx = Vx SHL 1.

    If the most-significant bit of Vx is 1, then VF is set 
    to 1, otherwise to 0. Then Vx is multiplied by 2.
*/

static void
op_8xyE(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;

    chip->v[0xF] = chip->v[x] >> 7; // most significant bit

    chip->v[x] <<= 1;
}

/*
    9xy0 - SNE Vx, Vy
    Skip next instruction if Vx != Vy.

    The values of Vx and Vy are compared, and if they are 
    not equal, the program counter is increased by 2.
*/

static void
op_9nnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if(chip->v[x] != chip->v[y])
        chip->pc += 2;
}

/*
    Annn - LD I, addr
    Set I = nnn.

    The value of register I is set to nnn.
*/

static void
op_Annn(Chip8 *chip, uint16_t opcode)
{
    uint16_t addr = opcode & 0x0FFF;

    chip->i = addr;
}

/*
    Bnnn - JP V0, addr
    Jump to location nnn + V0.

    The program counter is set to nnn plus the value of V0.
*/

static void
op_Bnnn(Chip8 *chip, uint16_t opcode)
{
    uint16_t addr = opcode & 0x0FFF;

    chip->pc = addr + chip->v[0];
}

/*
    Cxkk - RND Vx, byte
    Set Vx = random byte AND kk.

    The interpreter generates a random number from 0 to 255, 
    which is then ANDed with the value kk. The results are 
    stored in Vx. See instruction 8xy2 for more information on AND.
*/

static void
op_Cnnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;

    chip->v[x] = (rand() % 256) & kk;
}

/*
    Dxyn - DRW Vx, Vy, nibble
    Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

    The interpreter reads n bytes from memory, starting at 
    the address stored in I. These bytes are then displayed 
    as sprites on screen at coordinates (Vx, Vy). Sprites are 
    XORed onto the existing screen. If this causes any pixels 
    to be erased, VF is set to 1, otherwise it is set to 0. 
    If the sprite is positioned so part of it is outside the 
    coordinates of the display, it wraps around to the opposite 
    side of the screen. See instruction 8xy3 for more information 
    on XOR, and section 2.4, Display, for more information on 
    the Chip-8 screen and sprites.
*/

static void
op_Dnnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;
    uint8_t Vx = chip->v[x];
    uint8_t Vy = chip->v[y];

    chip->v[0xF] = 0;

    for(int i = 0; i < n; i++)
    {
        uint8_t byte = chip->memory[chip->i + i];

        for(int j = 0; j < 8; j++)
        {
            uint8_t pixel = (byte >> (7 - j)) & 1;

            int x_pos = (Vx + j) % 64;
            int y_pos = (Vy + i) % 32;

            if(pixel && chip->display[y_pos * 64 + x_pos])
                chip->v[0xF] = 1;
            
            chip->display[y_pos * 64 + x_pos] ^= pixel;
        }
    }
}

static void op_Ex9E(Chip8 *chip, uint16_t opcode);
static void op_ExA1(Chip8 *chip, uint16_t opcode);

static void
op_Ennn(Chip8 *chip, uint16_t opcode)
{
    uint8_t kk = opcode & 0x00FF;

    switch(kk)
    {
        case 0x9E:
            op_Ex9E(chip, opcode);
            break;
        
        case 0xA1:
            op_ExA1(chip, opcode);
            break;

        default:
            perror("Unknown E-Series instruction");
            break;
    }
}

/*
    Ex9E - SKP Vx
    Skip next instruction if key with the value of Vx is pressed.

    Checks the keyboard, and if the key corresponding to the value 
    of Vx is currently in the down position, PC is increased by 2.
*/

static void
op_Ex9E(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t Vx = chip->v[x];

    if(chip->keypad[Vx])
        chip->pc += 2;
}

/*
    ExA1 - SKNP Vx
    Skip next instruction if key with the value of Vx is not pressed.

    Checks the keyboard, and if the key corresponding to the value 
    of Vx is currently in the up position, PC is increased by 2.
*/

static void
op_ExA1(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t Vx = chip->v[x];

    if(!chip->keypad[Vx])
        chip->pc += 2;
}


static void op_Fx07(Chip8 *chip, uint16_t opcode);
static void op_Fx0A(Chip8 *chip, uint16_t opcode);
static void op_Fx15(Chip8 *chip, uint16_t opcode);
static void op_Fx18(Chip8 *chip, uint16_t opcode);
static void op_Fx1E(Chip8 *chip, uint16_t opcode);
static void op_Fx29(Chip8 *chip, uint16_t opcode);
static void op_Fx33(Chip8 *chip, uint16_t opcode);
static void op_Fx55(Chip8 *chip, uint16_t opcode);
static void op_Fx65(Chip8 *chip, uint16_t opcode);

static void
op_Fnnn(Chip8 *chip, uint16_t opcode)
{
    uint8_t kk = opcode & 0x00FF;

    switch(kk)
    {
        case 0x07: op_Fx07(chip, opcode); break;
        case 0x0A: op_Fx0A(chip, opcode); break;
        case 0x15: op_Fx15(chip, opcode); break; 
        case 0x18: op_Fx18(chip, opcode); break;
        case 0x1E: op_Fx1E(chip, opcode); break;
        case 0x29: op_Fx29(chip, opcode); break;
        case 0x33: op_Fx33(chip, opcode); break;
        case 0x55: op_Fx55(chip, opcode); break;
        case 0x65: op_Fx65(chip, opcode); break;
        default:
            perror("Unknown F-Series instruction");
            return;
    }
}

/*
    Fx07 - LD Vx, DT
    Set Vx = delay timer value.

    The value of DT is placed into Vx.
*/

static void
op_Fx07(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;

    chip->v[x] = chip->delay_timer;
}

/*
    Fx0A - LD Vx, K
    Wait for a key press, store the value of the key in Vx.

    All execution stops until a key is pressed, then the 
    value of that key is stored in Vx.
*/

static void
op_Fx0A(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;

    for(int i = 0; i < 16; i++)
        if(chip->keypad[i]){
            chip->v[x] = i;
            return;
        }
    
    // no key pressed
    chip->pc -= 2;
}

/*
    Fx15 - LD DT, Vx
    Set delay timer = Vx.

    DT is set equal to the value of Vx.
*/

static void
op_Fx15(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;

    chip->delay_timer = chip->v[x];
}

/*
    Fx18 - LD ST, Vx
    Set sound timer = Vx.

    ST is set equal to the value of Vx.
*/

static void
op_Fx18(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;

    chip->sound_timer = chip->v[x];
}

/*
    Fx1E - ADD I, Vx
    Set I = I + Vx.

    The values of I and Vx are added, and the results are stored in I.
*/

static void
op_Fx1E(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;

    chip->i += chip->v[x];
}

/*
    Fx29 - LD F, Vx
    Set I = location of sprite for digit Vx.

    The value of I is set to the location for the hexadecimal sprite 
    corresponding to the value of Vx. See section 2.4, Display, for 
    more information on the Chip-8 hexadecimal font.
*/

static void
op_Fx29(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t digit = chip->v[x];

    chip->i = FONTSET_START_ADDRESS + 5 * digit;
}

/*
    Fx33 - LD B, Vx
    Store BCD representation of Vx in memory locations I, I+1, and I+2.

    The interpreter takes the decimal value of Vx, and places the 
    hundreds digit in memory at location in I, the tens digit at 
    location I+1, and the ones digit at location I+2.
*/

static void
op_Fx33(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t Vx = chip->v[x];

    chip->memory[chip->i] = Vx / 100;
    chip->memory[chip->i + 1] = (Vx / 10) % 10;
    chip->memory[chip->i + 2] = Vx % 10;
}

/*
    Fx55 - LD [I], Vx
    Store registers V0 through Vx in memory starting at location I.

    The interpreter copies the values of registers V0 through Vx 
    into memory, starting at the address in I.
*/

static void
op_Fx55(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;

    memcpy(&chip->memory[chip->i], chip->v, x + 1);    
}

/*
    Fx65 - LD Vx, [I]
    Read registers V0 through Vx from memory starting at location I.

    The interpreter reads values from memory starting at 
    location I into registers V0 through Vx.
*/

static void
op_Fx65(Chip8 *chip, uint16_t opcode)
{
    uint8_t x = (opcode & 0x0F00) >> 8;

    memcpy(chip->v, &chip->memory[chip->i], x + 1);   
}
