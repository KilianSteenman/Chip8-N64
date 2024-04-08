//
// Created by Shadow-Link on 24/03/2024.
//

#ifndef CHIP_8_INPUT_H
#define CHIP_8_INPUT_H

#include "stdbool.h"
#include <libdragon.h>

#include "chip8.h"

extern const uint8_t KEY_BINDING_NOT_SET;

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

extern const char button_names[13][10];

typedef struct {
    uint8_t key[0x10];
} KeyMap;

bool is_button_pressed(struct controller_data controllers, int controller_index, Button button);

void update_button_states(C8_State *c8_state, KeyMap key_map, struct controller_data controllers);

void set_key_binding(KeyMap *key_map, int key, int controller_index, int button_index);

void init_key_map(KeyMap *key_map);

void store_key_map(char *name, KeyMap *key_map);

void load_key_map(uint8_t *buffer, KeyMap *key_map);

#endif //CHIP_8_INPUT_H
