//
// Created by Shadow-Link on 17/02/2024.
//

#include "chip8_cpu.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int PROGRAM_OFFSET = 0x200;
const int FONT_OFFSET = 0x050;

char fontArray[] = {
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

FILE *logFile;

void C8_print_state(C8_CPU_State *state) {
    int16_t programCounter;

    int16_t index;

    int16_t stack[16];
    int8_t stackIndex;

    uint8_t delayTimer;
    uint8_t soundTimer;
    uint8_t registers[16];
    fprintf(logFile, "%d, %d Timer[%d, %d], Reg[%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
            state->programCounter,
            state->index,
            state->delayTimer,
            state->soundTimer,
            state->registers[0],
            state->registers[1],
            state->registers[2],
            state->registers[3],
            state->registers[4],
            state->registers[5],
            state->registers[6],
            state->registers[7],
            state->registers[8],
            state->registers[9],
            state->registers[10],
            state->registers[11],
            state->registers[12],
            state->registers[13],
            state->registers[14],
            state->registers[15]
    );
}

void C8_print_display_state(C8_CPU_State *state) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            fprintf(logFile, "%d", state->display[i][j]);
        }
    }
    fprintf(logFile, "\n");
}

void C8_load_font(C8_CPU_State *state, char *font, char size) {
    memcpy(&state->memory[FONT_OFFSET], font, size);
}

void C8_load_program(C8_CPU_State *state, char *program, int programSize) {
    memcpy(&state->memory[PROGRAM_OFFSET], program, programSize);
    state->programCounter = PROGRAM_OFFSET;
}

void C8_init(C8_CPU_State *state) {
    logFile = fopen("test.log", "w"); // a (create) option will allow appending which is useful in a log file
    if (logFile == NULL) { exit(0); }

    memset(state->registers, 0, sizeof(state->registers));
    memset(state->memory, 0, sizeof(state->memory));
    memset(state->stack, 0, sizeof(state->stack));
    memset(state->display, 0, sizeof(state->display));
    memset(state->keys, 0, sizeof(state->keys));

    state->delayTimer = 60;
    state->soundTimer = 0;
    state->draw = 0;
    state->index = 0;

    C8_load_font(state, &fontArray, sizeof(fontArray));

    // Make sure random is actually random
    srand(time(NULL));
}

void C8_clear_screen(C8_CPU_State *state) {
    memset(state->display, 0, sizeof(state->display));
    state->draw = 1;
}

short C8_get_next_opcode(C8_CPU_State *state) {
    short opcode = 0;
    opcode = state->memory[state->programCounter] << 8;
    state->programCounter += 1;
    opcode |= state->memory[state->programCounter] << 0;
    state->programCounter += 1;
    return opcode;
}

void C8_push_stack(C8_CPU_State *state) {
    state->stack[state->stackIndex++] = state->programCounter;
}

short C8_pop_stack(C8_CPU_State *state) {
    return state->stack[--state->stackIndex];
}

void C8_opcode_00E0_clear_screen(C8_CPU_State *state) {
    C8_clear_screen(state);
}

void C8_opcode_00EE_subroutine_return(C8_CPU_State *state) {
    state->programCounter = C8_pop_stack(state);
    printf("PC %d\n", state->programCounter);
}

void C8_opcode_1XXX_set_pc(C8_CPU_State *state, short opcode) {
    short pc = opcode & 0x0FFF;
    state->programCounter = pc;
}

void C8_opcode_2XXX_subroutine(C8_CPU_State *state, short opcode) {
    short pc = opcode & 0x0FFF;
    C8_push_stack(state);
    state->programCounter = pc;
}

void C8_opcode_3XXX_skip_conditionally(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;

    printf("Checking register %d %d == %d\n", reg, state->registers[reg], value);
    if (state->registers[reg] == value) {
        state->programCounter += 2;
    }
}

void C8_opcode_4XXX_skip_conditionally(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;

    if (state->registers[reg] != value) {
        state->programCounter += 2;
    }
}

void C8_opcode_5XXX_skip_conditionally(C8_CPU_State *state, short opcode) {
    uint8_t regX = (opcode & 0x0F00) >> 8;
    uint8_t regY = (opcode & 0x00F0) >> 4;

    if (state->registers[regX] == state->registers[regY]) {
        state->programCounter += 2;
    }
}

void C8_opcode_6XXX_set_register(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;
    state->registers[reg] = value;
}

void C8_opcode_7XXX_add_value_to_register(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;
    state->registers[reg] += value;
}

void C8_opcode_8XX0_set(C8_CPU_State *state, short opcode) {
    uint8_t regX = (opcode & 0x0F00) >> 8;
    uint8_t regY = (opcode & 0x00F0) >> 4;

    state->registers[regX] = state->registers[regY];
}

void C8_opcode_8XX1_or(C8_CPU_State *state, short opcode) {
    uint8_t regX = (opcode & 0x0F00) >> 8;
    uint8_t regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[regX] = valueX | valueY;
}

void C8_opcode_8XX2_and(C8_CPU_State *state, short opcode) {
    uint8_t regX = (opcode & 0x0F00) >> 8;
    uint8_t regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[regX] = valueX & valueY;
}

void C8_opcode_8XX3_xor(C8_CPU_State *state, short opcode) {
    uint8_t regX = (opcode & 0x0F00) >> 8;
    uint8_t regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[regX] = valueX ^ valueY;
}

void C8_opcode_8XX4_add(C8_CPU_State *state, short opcode) {
    uint8_t regX = (opcode & 0x0F00) >> 8;
    uint8_t regY = (opcode & 0x00F0) >> 4;

    uint8_t valueX = state->registers[regX];
    uint8_t valueY = state->registers[regY];

    state->registers[regX] = valueX + valueY;
    state->registers[0xF] = state->registers[regX] < valueX;
}

void C8_opcode_8XX5_subtract(C8_CPU_State *state, short opcode) {
    uint8_t regX = (opcode & 0x0F00) >> 8;
    uint8_t regY = (opcode & 0x00F0) >> 4;

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
    uint8_t regX = (opcode & 0x0F00) >> 8;
//    uint8_t regY = (opcode & 0x00F0) >> 4; TODO: Is this actually unused?

    uint8_t valueX = state->registers[regX];
//    uint8_t valueY = state->registers[regY]; TODO: Is this actually unused?

    state->registers[0xF] = (valueX & 0x01) == 0x01;
    state->registers[regX] = valueX >> 1;
}

void C8_opcode_8XX7_subtract(C8_CPU_State *state, short opcode) {
    uint8_t regX = (opcode & 0x0F00) >> 8;
    uint8_t regY = (opcode & 0x00F0) >> 4;

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
    uint8_t regX = (opcode & 0x0F00) >> 8;
//    uint8_t regY = (opcode & 0x00F0) >> 4; TODO: Is this actually unused?

    uint8_t valueX = state->registers[regX];
//  uint8_t valueY = state->registers[regY]; TODO: Is this actually unused?

    state->registers[0xF] = (valueX & 0x80) == 0x80;
    state->registers[regX] = valueX << 1;
}

void C8_opcode_9XXX_skip_conditionally(C8_CPU_State *state, short opcode) {
    uint8_t regX = (opcode & 0x0F00) >> 8;
    uint8_t regY = (opcode & 0x00F0) >> 4;

    if (state->registers[regX] != state->registers[regY]) {
        state->programCounter += 2;
    }
}

void C8_opcode_AXXX_set_index(C8_CPU_State *state, short opcode) {
    short value = opcode & 0x0FFF;
    state->index = value;
}

void C8_opcode_BXXX_jump_with_offset(C8_CPU_State *state, short opcode) {
    short value = opcode & 0x0FFF;
    state->programCounter = value + state->registers[0];
}

void C8_opcode_CXXX_random(C8_CPU_State *state, short opcode) {
    uint8_t regX = opcode & 0x0F00 >> 8;
    uint8_t value = opcode & 0x00FF;

    uint8_t random = rand();
    state->registers[regX] = random & value;
}

void C8_opcode_DXXX_display(C8_CPU_State *state, short opcode) {
    uint8_t sprite_height = opcode & 0x000F;
    uint8_t x = state->registers[(opcode & 0x0F00) >> 8] % 64;
    uint8_t y = state->registers[(opcode & 0x00F0) >> 4] % 32;
    uint8_t pixel;

    state->registers[0xF] = 0;

    for (int y_coordinate = 0; y_coordinate < sprite_height; y_coordinate++) {
        pixel = state->memory[state->index + y_coordinate];
        for (int x_coordinate = 0; x_coordinate < 8; x_coordinate++) {
            if ((pixel & (0x80 >> x_coordinate)) != 0) {
                if (state->display[y + y_coordinate][x + x_coordinate] == 1) {
                    state->registers[0xF] = 1;
                }
                state->display[y + y_coordinate][x + x_coordinate] ^= 1;
                fprintf(logFile, "[%d] %d at (%d,%d)\n", (((y + y_coordinate) * 64) + (x + x_coordinate)),
                        state->display[y + y_coordinate][x + x_coordinate], x, y);
            }
        }
    }
    state->draw = 1;
}

void C8_opcode_EX9E_is_key_pressed(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    if (state->keys[reg] == 1) {
        state->programCounter += 2;
    }
}

void C8_opcode_EXA1_is_key_not_pressed(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    if (state->keys[reg] == 0) {
        state->programCounter += 2;
    }
}

void C8_opcode_FX07_store_delay_timer(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    state->registers[reg] = state->delayTimer;
}

void C8_opcode_FX15_set_delay_timer(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    state->delayTimer = state->registers[reg];
}

void C8_opcode_FX18_set_sound_timer(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    state->soundTimer = state->registers[reg];
}

void C8_opcode_FX1E_add_to_index(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    state->index += state->registers[reg];
}

void C8_opcode_FXXX_load_font_char(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    state->index = (FONT_OFFSET + (state->registers[reg] * 5));
}

void C8_opcode_FX33_bin_dec(C8_CPU_State *state, short opcode) {
    uint8_t reg = (opcode & 0x0F00) >> 8;
    uint8_t value = state->registers[reg];

    uint8_t p1 = (value / 100);
    uint8_t p2 = (value / 10 % 10);
    uint8_t p3 = (value % 100 % 10);

    printf("Value %d, %d %d %d\n", value, p1, p2, p3);

    state->memory[state->index] = p1;
    state->memory[state->index + 1] = p2;
    state->memory[state->index + 2] = p3;
}

void C8_opcode_FX55_store_mem(C8_CPU_State *state, short opcode) {
    uint8_t count = (opcode & 0x0F00) >> 8;
    printf("Storing %d\n", count);
    for (int i = 0; i <= count; i++) {
        printf("Store[%d] %d into %d\n", i, state->registers[i], (state->index + 1));
        state->memory[state->index + i] = state->registers[i];
    }
}

void C8_opcode_FX65_read_mem(C8_CPU_State *state, short opcode) {
    uint8_t count = (opcode & 0x0F00) >> 8;
    printf("Reading %d\n", count);
    for (int i = 0; i <= count; i++) {
        printf("Read[%d] %d\n", i, state->memory[state->index + i]);
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
    } else if ((opcode & 0xF00F) == 0x8000) {
        C8_opcode_8XX0_set(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8001) {
        C8_opcode_8XX1_or(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8002) {
        C8_opcode_8XX2_and(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8003) {
        C8_opcode_8XX3_xor(state, opcode);
    } else if ((opcode & 0xF00F) == 0x8004) {
        C8_opcode_8XX4_add(state, opcode);
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
    } else if ((opcode & 0xF000) == 0xB000) {
        C8_opcode_BXXX_jump_with_offset(state, opcode);
    } else if ((opcode & 0xF000) == 0xC000) {
        C8_opcode_CXXX_random(state, opcode);
    } else if ((opcode & 0xF000) == 0xD000) {
        C8_opcode_DXXX_display(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xE09E) {
        C8_opcode_EX9E_is_key_pressed(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xE0A1) {
        C8_opcode_EXA1_is_key_not_pressed(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xF007) {
        C8_opcode_FX07_store_delay_timer(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xF015) {
        C8_opcode_FX15_set_delay_timer(state, opcode);
    } else if ((opcode & 0xF0FF) == 0xF018) {
        C8_opcode_FX18_set_sound_timer(state, opcode);
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

    // Update timers
    if (state->delayTimer > 0) {
        state->delayTimer--;
    }
    if (state->soundTimer > 0) {
        state->soundTimer--;
    }

//    C8_print_state(state);
}