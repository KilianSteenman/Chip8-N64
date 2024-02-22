//
// Created by Shadow-Link on 17/02/2024.
//

#include "chip8_cpu.h"

#include <string.h>
#include <stdio.h>

const int PROGRAM_OFFSET = 0x200;

void C8_load_program(C8_CPU_State *state, char *program, int programSize) {
    memcpy(&state->memory[PROGRAM_OFFSET], program, programSize);
    state->programCounter = PROGRAM_OFFSET;
}

short C8_get_next_opcode(C8_CPU_State *state) {
    return (state->memory[state->programCounter++] << 8) | (state->memory[state->programCounter++] << 0);
}

void C8_opcode_00E0_clear_screen(C8_CPU_State *state) {
    printf("Clear screen\n");
    for(int y = 0; y < 32; y++) {
        for(int x = 0; x < 64; x++) {
            state->display[y][x] = 0;
        }
    }
}

void C8_opcode_1XXX_set_pc(C8_CPU_State *state, short opcode) {
    short pc = opcode & 0x0FFF;
    printf("Set pc: %d\n", pc);
    state->programCounter = pc;
}

void C8_opcode_6XXX_set_register(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    char value = opcode & 0x00FF;
    printf("Set reg %d to %d\n", reg, value);
    state->registers[reg] = value;
}

void C8_opcode_7XXX_add_value_to_register(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    char value = opcode & 0x00FF;
    state->registers[reg] += value;
    printf("Add %d to reg %d result %d\n", reg, value, state->registers[reg]);
}

void C8_opcode_AXXX_set_index(C8_CPU_State *state, short opcode) {
    short value = opcode & 0x0FFF;
    state->index = value;
    printf("Set index to %d\n", value);
}

void C8_opcode_DXXX_display(C8_CPU_State *state, short opcode) {
    char sprite_height = opcode & 0x000F;
    char x = state->registers[(opcode & 0x0100) >> 8] % 64;
    char y = state->registers[(opcode & 0x0010) >> 4] % 32;
    char pixel;

    state->registers[0xF] = 0;

    for (int y_coordinate = 0; y_coordinate < sprite_height; y_coordinate++) {
        pixel = state->memory[state->index + y_coordinate];
        for (int x_coordinate = 0; x_coordinate < 8; x_coordinate++) {
            if ((pixel & (0x80 >> x_coordinate)) != 0) {
                if (state->display[y + y_coordinate][x + x_coordinate] == 1) {
                    state->registers[0xF] = 1;
                }
                state->display[y + y_coordinate][x + x_coordinate] ^= 1;
            }
        }
    }
}

void C8_execute_opcode(C8_CPU_State *state, short opcode) {
    printf("Opcode: %02X\n", opcode);
    if (opcode == 0x00E0) {
        C8_opcode_00E0_clear_screen(state);
    } else if ((opcode & 0xF000) == 0x1000) {
        C8_opcode_1XXX_set_pc(state, opcode);
    } else if ((opcode & 0xF000) == 0x6000) {
        C8_opcode_6XXX_set_register(state, opcode);
    } else if ((opcode & 0xF000) == 0x7000) {
        C8_opcode_7XXX_add_value_to_register(state, opcode);
    } else if ((opcode & 0xF000) == 0xA000) {
        C8_opcode_AXXX_set_index(state, opcode);
    } else if ((opcode & 0xF000) == 0xD000) {
        C8_opcode_DXXX_display(state, opcode);
    } else {
        printf("Unknown Opcode: %02X\n", opcode);
    }
}

void C8_execute_program(C8_CPU_State *state) {
    short opcode = C8_get_next_opcode(state);
    C8_execute_opcode(state, opcode);
}
