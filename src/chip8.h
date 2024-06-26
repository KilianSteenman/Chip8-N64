//
// Created by Shadow-Link on 17/02/2024.
//

#ifndef CHIP_8_CHIP8_H
#define CHIP_8_CHIP8_H

#include <stdint.h>
#include "rom.h"

typedef struct {
    uint8_t keys[16];
    uint8_t draw;

    int16_t programCounter;

    int16_t index;

    int16_t stack[16];
    int8_t stackIndex;

    uint8_t delayTimer;
    uint8_t soundTimer;
    uint8_t registers[16];
    uint8_t memory[4096];

    uint8_t display[32][64];
} C8_State;

void C8_init(C8_State *state);
void C8_load_program(C8_State *state, Rom *rom);
void C8_execute_program(C8_State* state);

#endif //CHIP_8_CHIP8_H
