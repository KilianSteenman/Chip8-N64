//
// Created by Shadow-Link on 08/04/2024.
//

#ifndef CHIP_8_SCREEN_ROM_EXECUTION_H
#define CHIP_8_SCREEN_ROM_EXECUTION_H

#include "chip8.h"
#include "input.h"

bool execute_rom(C8_State *cpu_state, struct controller_data controllers, KeyMap *key_map);

#endif //CHIP_8_SCREEN_ROM_EXECUTION_H
