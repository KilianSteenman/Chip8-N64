//
// Created by Shadow-Link on 08/04/2024.
//

#include <stdio.h>
#include <string.h>
#include <libdragon.h>

#include "screen_rom_select.h"

bool execute_rom_select(char rom_files[][30], int rom_count, int *selected_game_index) {
    console_set_render_mode(RENDER_MANUAL);
    console_clear();
    for (int i = 0; i < rom_count; i++) {
        if (i == *selected_game_index) {
            printf("- %s\n", rom_files[i]);
        } else {
            printf("%s\n", rom_files[i]);
        }
    }
    console_render();

    struct controller_data controllers = get_keys_down();
    if (controllers.c[0].up) {
        if (--*selected_game_index < 0) {
            *selected_game_index = 0;
        }
    }

    if (controllers.c[0].down) {
        if (++*selected_game_index >= rom_count) {
            *selected_game_index = rom_count - 1;
        }
    }

    return controllers.c[0].A;
}
