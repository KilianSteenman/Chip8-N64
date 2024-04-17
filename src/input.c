//
// Created by Shadow-Link on 24/03/2024.
//

#include "input.h"
#include "chip8.h"

const uint8_t KEY_BINDING_NOT_SET = 0xFF;

const char button_names[13][10] = {
        "A",
        "B",
        "L",
        "R",
        "Z",
        "Up",
        "Down",
        "Left",
        "Right",
        "C-Up",
        "C-Down",
        "C-Left",
        "C-Right"
};

bool is_button_pressed(struct controller_data controllers, int controller_index, Button button) {
    switch (button) {
        case A:
            return controllers.c[controller_index].A;
            break;
        case B:
            return controllers.c[controller_index].B;
            break;
        case L:
            return controllers.c[controller_index].L;
            break;
        case R:
            return controllers.c[controller_index].R;
            break;
        case Z:
            return controllers.c[controller_index].Z;
            break;
        case Up:
            return controllers.c[controller_index].up;
            break;
        case Down:
            return controllers.c[controller_index].down;
            break;
        case Left:
            return controllers.c[controller_index].left;
            break;
        case Right:
            return controllers.c[controller_index].right;
            break;
        case C_Up:
            return controllers.c[controller_index].C_up;
            break;
        case C_Down:
            return controllers.c[controller_index].C_down;
            break;
        case C_Left:
            return controllers.c[controller_index].C_left;
            break;
        case C_Right:
            return controllers.c[controller_index].C_right;
            break;
        default:
            printf("Unknown button");
            exit(0);
            break;
    }
}

void update_button_states(C8_State *c8_state, KeyMap key_map, struct controller_data controllers) {
    for (int i = 0; i < sizeof(key_map.key); i++) {
        if (key_map.key[i] != KEY_BINDING_NOT_SET) {
            c8_state->keys[i] = is_button_pressed(controllers, key_map.key[i] & 0xF, key_map.key[i] >> 4 & 0xF);
        }
    }
}

void set_key_binding(KeyMap *key_map, int key, int controller_index, int button_index) {
    uint8_t binding = (button_index << 4) | controller_index;
    key_map->key[key] = binding;
}

void init_key_map(KeyMap *key_map) {
    for(int i = 0; i < sizeof(key_map->key); i++) {
        key_map->key[i] = KEY_BINDING_NOT_SET;
    }
}

void store_key_map(char *name, KeyMap *key_map) {
    // Create and allocate memory for an 32-byte buffer
    uint8_t *buffer = malloc(32 * sizeof(uint8_t));
    // Use the first 16 bytes to store the filename
    for (int i = 0; i < 16; i++) {
        buffer[i] = name[i];
    }
    // Use the second 16 bytes to store the keymap
    for (int i = 0; i < 16; i++) {
        buffer[i + 16] = key_map->key[i];
    }
    eeprom_write_bytes(buffer, 0, 32);
    free(buffer);
}

void load_key_map(uint8_t *buffer, KeyMap *key_map) {
    // Use the second 16 bytes to load the keymap
    for (int i = 0; i < 16; i++) {
        key_map->key[i] = buffer[i];
    }
}
