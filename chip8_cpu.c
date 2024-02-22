//
// Created by Shadow-Link on 17/02/2024.
//

#include "chip8_cpu.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const int PROGRAM_OFFSET = 0x200;
const int FONT_OFFSET = 0x050;

void C8_load_font(C8_CPU_State *state, char *font, char size) {
    memcpy(&state->memory[FONT_OFFSET], font, size);
}

void C8_load_program(C8_CPU_State *state, char *program, int programSize) {
    memcpy(&state->memory[PROGRAM_OFFSET], program, programSize);
    state->programCounter = PROGRAM_OFFSET;
}

short C8_get_next_opcode(C8_CPU_State *state) {
    return (state->memory[state->programCounter++] << 8) | (state->memory[state->programCounter++] << 0);
}

void C8_push_stack(C8_CPU_State *state) {
    state->stack[state->stackIndex++] = state->programCounter;
}

short C8_pop_stack(C8_CPU_State *state) {
    return state->stack[state->stackIndex--];
}

void C8_opcode_00E0_clear_screen(C8_CPU_State *state) {
//    printf("Clear screen\n");
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            state->display[y][x] = 0;
        }
    }
}

void C8_opcode_00EE_subroutine_return(C8_CPU_State *state) {
    state->programCounter = C8_pop_stack(state);
}

void C8_opcode_1XXX_set_pc(C8_CPU_State *state, short opcode) {
    short pc = opcode & 0x0FFF;
//    printf("Set pc: %d\n", pc);
    state->programCounter = pc;
}

void C8_opcode_2XXX_subroutine(C8_CPU_State *state, short opcode) {
    short pc = opcode & 0x0FFF;
//    printf("Set pc: %d\n", pc);
    C8_push_stack(state);
    state->programCounter = pc;
}

void C8_opcode_3XXX_skip_conditionally(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;

    printf("Checking register %d %d == %d\n", reg, state->registers[reg], value);
    if (state->registers[reg] == value) {
        state->programCounter += 2;
    }
}

void C8_opcode_4XXX_skip_conditionally(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;

    if (state->registers[reg] != value) {
        state->programCounter += 2;
    }
}

void C8_opcode_5XXX_skip_conditionally(C8_CPU_State *state, short opcode) {
    char regX = (opcode & 0x0F00) >> 8;
    char regY = (opcode & 0x00F0) >> 4;

    if (state->registers[regX] == state->registers[regY]) {
        state->programCounter += 2;
    }
}

void C8_opcode_6XXX_set_register(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;
//    printf("Set reg %d to %d\n", reg, value);
    state->registers[reg] = value;
}

void C8_opcode_7XXX_add_value_to_register(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;
    state->registers[reg] += value;
//    printf("Add %d to reg %d result %d\n", reg, value, state->registers[reg]);
}

void C8_opcode_8XX1_or(C8_CPU_State *state, short opcode) {
    char regX = (opcode & 0x0F00) >> 8;
    char regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[regX] = valueX | valueY;
}

void C8_opcode_8XX2_and(C8_CPU_State *state, short opcode) {
    char regX = (opcode & 0x0F00) >> 8;
    char regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[regX] = valueX & valueY;
}

void C8_opcode_8XX3_xor(C8_CPU_State *state, short opcode) {
    char regX = (opcode & 0x0F00) >> 8;
    char regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[regX] = valueX ^ valueY;
}

void C8_opcode_8XX5_subtract(C8_CPU_State *state, short opcode) {
    char regX = (opcode & 0x0F00) >> 8;
    char regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[regX] = valueX - valueY;

    if (valueX > valueY) {
        state->registers[0xF] = 1;
    } else {
        state->registers[0xF] = 0;
    }
}

void C8_opcode_8XX6_shift_right(C8_CPU_State *state, short opcode) {
    char regX = (opcode & 0x0F00) >> 8;
    char regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[0xF] = (valueX & 0x01) == 0x01;
    state->registers[regX] = valueX >> 1;
}

void C8_opcode_8XX7_subtract(C8_CPU_State *state, short opcode) {
    char regX = (opcode & 0x0F00) >> 8;
    char regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[regX] = valueY - valueX;

    if (valueY > valueX) {
        state->registers[0xF] = 1;
    } else {
        state->registers[0xF] = 0;
    }
}

void C8_opcode_8XXE_shift_left(C8_CPU_State *state, short opcode) {
    char regX = (opcode & 0x0F00) >> 8;
    char regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[0xF] = (valueX & 0x80) == 0x80;
    state->registers[regX] = valueX << 1;
}

void C8_opcode_9XXX_skip_conditionally(C8_CPU_State *state, short opcode) {
    char regX = (opcode & 0x0F00) >> 8;
    char regY = (opcode & 0x00F0) >> 4;

    if (state->registers[regX] != state->registers[regY]) {
        state->programCounter += 2;
    }
}

void C8_opcode_AXXX_set_index(C8_CPU_State *state, short opcode) {
    short value = opcode & 0x0FFF;
    state->index = value;
//    printf("Set index to %d\n", value);
}

void C8_opcode_DXXX_display(C8_CPU_State *state, short opcode) {
    char sprite_height = opcode & 0x000F;
    uint8_t x = state->registers[(opcode & 0x0100) >> 8] % 64;
    uint8_t y = state->registers[(opcode & 0x0010) >> 4] % 32;
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

void C8_opcode_FX15_set_delay_timer(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    state->delayTimer = state->registers[reg];
}

void C8_opcode_FX1E_add_to_index(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    state->index += state->registers[reg];
}

void C8_opcode_FXXX_load_font_char(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    state->index = (FONT_OFFSET + (state->registers[reg] * 5));
}

void C8_opcode_FX33_bin_dec(C8_CPU_State *state, short opcode) {
    char reg = (opcode & 0x0F00) >> 8;
    uint8_t value = state->registers[reg];

    state->memory[state->index] = (value / 100);
    state->memory[state->index + 1] = (value / 10 % 10);
    state->memory[state->index + 2] = (value % 100 % 10);
}

void C8_opcode_FX55_store_mem(C8_CPU_State *state, short opcode) {
    char count = (opcode & 0x0F00) >> 8;
    for(int i = 0; i < count; i++) {
        state->memory[state->index + i] = state->registers[i];
    }
}

void C8_opcode_FX65_read_mem(C8_CPU_State *state, short opcode) {
    char count = (opcode & 0x0F00) >> 8;
    for(int i = 0; i < count; i++) {
        state->registers[i] = state->memory[state->index + i];
    }
}

void C8_execute_opcode(C8_CPU_State *state, short opcode) {
    printf("Opcode: %02X\n", opcode);
    if (opcode == 0x00E0) {
        C8_opcode_00E0_clear_screen(state);
    } else if (opcode == 0x00EE) {
        C8_opcode_00EE_subroutine_return(state);
    } else if ((opcode & 0xF000) == 0x1000) {
        C8_opcode_1XXX_set_pc(state, opcode);
    } else if ((opcode & 0xF000) == 0x2000) {
        C8_opcode_2XXX_subroutine(state, opcode);
    } else if ((opcode & 0xF000) == 0x3000) {
        C8_opcode_3XXX_skip_conditionally(state, opcode);
    } else if ((opcode & 0xF000) == 0x4000) {
        C8_opcode_4XXX_skip_conditionally(state, opcode);
    } else if ((opcode & 0xF000) == 0x5000) {
        C8_opcode_5XXX_skip_conditionally(state, opcode);
    } else if ((opcode & 0xF000) == 0x6000) {
        C8_opcode_6XXX_set_register(state, opcode);
    } else if ((opcode & 0xF000) == 0x7000) {
        C8_opcode_7XXX_add_value_to_register(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8001) {
        C8_opcode_8XX1_or(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8002) {
        C8_opcode_8XX2_and(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8003) {
        C8_opcode_8XX3_xor(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8005) {
        C8_opcode_8XX5_subtract(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8006) {
        C8_opcode_8XX6_shift_right(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8007) {
        C8_opcode_8XX7_subtract(state, opcode);
    } else if ((opcode & 0xF00F) == 0x800E) {
        C8_opcode_8XXE_shift_left(state, opcode);
    } else if ((opcode & 0xF000) == 0x9000) {
        C8_opcode_9XXX_skip_conditionally(state, opcode);
    } else if ((opcode & 0xF000) == 0xA000) {
        C8_opcode_AXXX_set_index(state, opcode);
    } else if ((opcode & 0xF000) == 0xD000) {
        C8_opcode_DXXX_display(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xF015) {
        C8_opcode_FX15_set_delay_timer(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xF01E) {
        C8_opcode_FX1E_add_to_index(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xF029) {
        C8_opcode_FXXX_load_font_char(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xF033) {
        C8_opcode_FX33_bin_dec(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xF055) {
        C8_opcode_FX55_store_mem(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xF065) {
        C8_opcode_FX65_read_mem(state, opcode);
    } else {
        printf("Unknown Opcode: %02X\n", opcode);
        exit(0);
    }
}

void C8_execute_program(C8_CPU_State *state) {
    short opcode = C8_get_next_opcode(state);
    C8_execute_opcode(state, opcode);
}