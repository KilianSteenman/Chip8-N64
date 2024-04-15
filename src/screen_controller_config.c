//
// Created by Shadow-Link on 08/04/2024.
//

#include <stdio.h>
#include <string.h>
#include <libdragon.h>

#include "screen_controller_config.h"

int selected_button_index = 0;
int is_in_config_mode = 0;

bool execute_controller_config(KeyMap *key_map) {
    console_set_render_mode(RENDER_MANUAL);
    console_clear();
    printf("controller config\n");
    printf("Input\tController\tButton\n");
    for (int i = 0; i < sizeof(key_map->key); i++) {
        if (key_map->key[i] == KEY_BINDING_NOT_SET) {
            if (i == selected_button_index) {
                printf("- %X\t\t%s\t\t\t%s\n", i, "-", "-");
            } else {
                printf("  %X\t\t%s\t\t\t%s\n", i, "-", "-");
            }
        } else {
            int controller_index = key_map->key[i] & 0xF;
            int button_index = key_map->key[i] >> 4;

            if (i == selected_button_index) {
                printf("- %X\t\t%d\t\t\t%s\n", i, controller_index, button_names[button_index]);
            } else {
                printf("  %X\t\t%d\t\t\t%s\n", i, controller_index, button_names[button_index]);
            }
        }
    }

    if (is_in_config_mode == 0) {
        struct controller_data controllers = get_keys_down();
        if (controllers.c[0].up) {
            if (--selected_button_index < 0) {
                selected_button_index = 0;
            }
        }

        if (controllers.c[0].down) {
            if (++selected_button_index > 0xF) {
                selected_button_index = 0xF;
            }
        }

        if (controllers.c[0].A) {
            is_in_config_mode = 1;
        }
        console_render();

        return controllers.c[0].start;
    } else {
        printf("\nPress a button\n");

        // Wait for button input
        struct controller_data controllers = get_keys_down();
        for (int controller_index = 0; controller_index < 4; controller_index++) {
            for (int button_index = 0; button_index < 12; button_index++) {
                if (is_button_pressed(controllers, controller_index, button_index)) {
                    set_key_binding(key_map, selected_button_index, controller_index, button_index);
                    is_in_config_mode = 0;
                    break;
                }
            }
        }
        console_render();
    }

    return false;
}
