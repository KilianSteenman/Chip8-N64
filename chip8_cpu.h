//
// Created by Shadow-Link on 17/02/2024.
//

#ifndef CHIP_8_CHIP8_H
#define CHIP_8_CHIP8_H

#include <stdint.h>

typedef struct {
    int16_t programCounter;

    int16_t index;
    // Assuming ArrayDeque is a dynamic array structure
    // You may need to implement it separately in C
    int16_t* stack;
    int16_t stack_capacity; // Capacity of the stack

    uint8_t delayTimer;
    uint8_t soundTimer;
    uint8_t registers[16];
    uint8_t memory[4096];

    uint8_t display[64*32];
} C8_CPU_State;

void C8_load_program(C8_CPU_State* state, char* program, int programSize);
void C8_execute_program(C8_CPU_State* state);

#endif //CHIP_8_CHIP8_H
