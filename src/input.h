//
// Created by Shadow-Link on 24/03/2024.
//

#ifndef CHIP_8_INPUT_H
#define CHIP_8_INPUT_H

#include "stdbool.h"
#include <libdragon.h>

typedef enum {
    A,
    B,
    L,
    R,
    Z,
    Up,
    Down,
    Left,
    Right,
    C_Up,
    C_Down,
    C_Left,
    C_Right
} Button;

typedef struct {
    uint8_t key[0xF];
} KeyMap;

bool is_button_pressed(struct controller_data controllers, int controller_index, Button button);

#endif //CHIP_8_INPUT_H
