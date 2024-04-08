//
// Created by Shadow-Link on 08/04/2024.
//

#include <stdio.h>
#include <string.h>
#include <libdragon.h>

#include "screen_rom_execution.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define GRID_SIZE 1 // Size of each grid cell
#define GRID_SCALE 1

void draw_display(C8_State *state, display_context_t disp) {
    uint32_t color_off = graphics_make_color(0, 0, 0, 255);
    uint32_t color_on = graphics_make_color(255, 255, 255, 255);

    for (int x = 0; x < SCREEN_WIDTH; x += GRID_SIZE) {
        for (int y = 0; y < SCREEN_HEIGHT; y += GRID_SIZE) {

            int xOffset = x / GRID_SCALE;
            int yOffset = y / GRID_SCALE;

            uint32_t color;
            if (state->display[yOffset][xOffset]) {
                color = color_on;
            } else {
                color = color_off;
            }
            graphics_draw_pixel(disp, x + 350, y, color);
        }
    }
}

bool execute_rom(C8_State *cpu_state, struct controller_data controllers, KeyMap *key_map) {
    C8_execute_program(cpu_state);

    display_context_t disp;
    if (cpu_state->draw == 1) {
        while (!(disp = display_get()));
        draw_display(cpu_state, disp);
        display_show(disp);
        cpu_state->draw = 0;
    }

    // Update key states
    controllers = get_keys_pressed();
    update_button_states(cpu_state, *key_map, controllers);

    // Start menu
    controllers = get_keys_down();
    return controllers.c[0].start;
}
