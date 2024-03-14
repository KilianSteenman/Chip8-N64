#include <stdio.h>
#include <libdragon.h>

#include "file_utils.h"
#include "chip8_cpu.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define GRID_SIZE 1 // Size of each grid cell
#define GRID_SCALE 1

void draw_display(C8_CPU_State *state, display_context_t disp) {
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
            graphics_draw_pixel(disp, x, y, color);
        }
    }
}

int main(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
    console_init();

    struct controller_data controllers;
    controller_init();

    debug_init_usblog();
//    console_set_debug(true);

    printf("Hello N64!\n");

    if (dfs_init(DFS_DEFAULT_LOCATION) != DFS_ESUCCESS) {
        printf("Filesystem failed to start!\n");
        return 1;
    }

    // Load Rom (TODO: Add rom selection screen)
    FILE *rom;
    char *buffer;
    long fileLength;

//        rom = fopen("rom://1-chip8-logo.ch8", "r");
//    rom = fopen("rom://2-ibm-logo.ch8", "r");
//    rom = fopen("rom://3-corax+.ch8", "r");
//    rom = fopen("rom://4-flags.ch8", "r");
    rom = fopen("rom://6-keypad.ch8", "r");
//    rom = fopen("rom://BC_test.ch8", "r");
//    rom = fopen("rom://Tetris.ch8", "r");
    if (rom == NULL) {
        printf("Unable to open game rom\n");
        return 1;
    }
    printf("Opened game rom\n");

    // Get the size of the rom
    fileLength = getFileSize(rom);

    // Allocate memory for the buffer to store file contents
    buffer = (char *) malloc(fileLength);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(rom);
        return 1;
    }
    printf("Allocated rom memory %ld\n", fileLength);

    // Read file contents into the buffer
    fread(buffer, 1, fileLength, rom);

    // Cleanup
    fclose(rom);

    // Init C8
    C8_CPU_State cpu_state;
    printf("Initializing C8\n");
    C8_init(&cpu_state);
    printf("Load program C8\n");
    C8_load_program(&cpu_state, buffer, fileLength);

    display_context_t disp;
    while (1) {
        C8_execute_program(&cpu_state);

        if (cpu_state.draw == 1) {
            while (!(disp = display_get()));
            draw_display(&cpu_state, disp);
            display_show(disp);
            cpu_state.draw = 0;
        }

        // Controller check
        controller_scan();
        controllers = get_keys_pressed();
        cpu_state.keys[0x4] = controllers.c[0].C_up;
        cpu_state.keys[0x1] = controllers.c[0].C_down;
        cpu_state.keys[0xE] = controllers.c[0].up;
        cpu_state.keys[0xF] = controllers.c[0].down;
        cpu_state.keys[0xA] = controllers.c[0].A;
//        printf("KEYS %d %d %d\n", cpu_state.keys[0xE], cpu_state.keys[0xF], cpu_state.keys[0xA]);
    }
}