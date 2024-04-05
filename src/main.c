#include <stdio.h>
#include <string.h>
#include <libdragon.h>

#include "rom.h"
#include "file_utils.h"
#include "chip8.h"
#include "input.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define GRID_SIZE 1 // Size of each grid cell
#define GRID_SCALE 1

typedef enum {
    GAME_SELECT,
    CONTROLLER_SETUP,
    GAME,
} State;

State state = GAME_SELECT;

char romFiles[8][30] = {
        "rom://1-chip8-logo.ch8",
        "rom://2-ibm-logo.ch8",
        "rom://3-corax+.ch8",
        "rom://4-flags.ch8",
        "rom://6-keypad.ch8",
        "rom://BC_test.ch8",
        "rom://Tetris.ch8",
        "rom://Pong.ch8"
};

int selected_game_index = 0;

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

void replace_extension(char *filepath, const char *new_extension) {
    char *dot = strrchr(filepath, '.');
    if (dot != NULL) {
        // Found the extension, replace it with the new one
        strcpy(dot + 1, new_extension);
    } else {
        // No extension found, append the new extension
        strcat(filepath, ".");
        strcat(filepath, new_extension);
    }
}

void load_controller_config(char *name) {
    replace_extension(name, "c8s");

    FILE *configFile;
    configFile = fopen(name, "r");
    if (configFile == NULL) {
        printf("Unable to open config file\n");
        state = CONTROLLER_SETUP;
        return;
    }

    printf("Loading settings %s\n", name);

    fclose(configFile);
}

Rom *load_rom(char *name) {
    FILE *romFile;
    char *buffer;
    long fileLength;

    romFile = fopen(name, "r");
    if (romFile == NULL) {
        printf("Unable to open game romFile\n");
        return NULL;
    }
    printf("Opened game %s\n", name);

    // Get the size of the romFile
    fileLength = getFileSize(romFile);

    // Allocate memory for the buffer to store file contents
    buffer = (char *) malloc(fileLength);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(romFile);
        return NULL;
    }
    printf("Allocated romFile memory %ld\n", fileLength);

    // Read file contents into the buffer
    fread(buffer, 1, fileLength, romFile);

    // Cleanup
    fclose(romFile);

    Rom *rom = malloc(sizeof(Rom));
    rom->buffer = buffer;
    rom->fileLength = fileLength;
    return rom;
}

void on_game_selected(C8_State *cpu_state, char *romFile) {
    console_clear();
    console_set_render_mode(RENDER_AUTOMATIC);

    printf("Loading rom %s\n", romFile);

    Rom *rom = load_rom(romFile);
    if (rom == NULL) {
        printf("Unable to load rom!\n");
        return;
    }

    C8_load_program(cpu_state, rom);
    free(rom);

    load_controller_config(romFile);
}

int selected_button_index = 0;
int is_in_config_mode = 0;

KeyMap key_map;

void execute_controller_config() {
    console_set_render_mode(RENDER_MANUAL);
    console_clear();
    printf("controller config\n");
    printf("Input\tController\tButton\n");
    for (int i = 0; i < 0xF; i++) {
        int controller_index = key_map.key[i] & 0xF;
        int button_index = key_map.key[i] >> 4;

        if (i == selected_button_index) {
            printf("- %X\t\t%d\t\t\t%s\n", i, controller_index, button_names[button_index]);
        } else {
            printf("  %X\t\t%d\t\t\t%s\n", i, controller_index, button_names[button_index]);
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

        if (controllers.c[0].start) {
            state = GAME;
        }
    } else {
        struct controller_data controllers = get_keys_down();
        for (int controller_index = 0; controller_index < 4; controller_index++) {
            for (int button_index = 0; button_index < 12; button_index++) {
                if (is_button_pressed(controllers, controller_index, button_index)) {
                    set_key_binding(&key_map, selected_button_index, controller_index, button_index);
                    is_in_config_mode = 0;
                    break;
                }
            }
        }
    }

    console_render();
}

void execute_game_select(C8_State *cpu_state) {
    console_set_render_mode(RENDER_MANUAL);
    console_clear();
    int romFileCount = sizeof(romFiles) / sizeof(romFiles[0]);
    for (int i = 0; i < romFileCount; i++) {
        if (i == selected_game_index) {
            printf("- %s\n", romFiles[i]);
        } else {
            printf("%s\n", romFiles[i]);
        }
    }
    console_render();

    struct controller_data controllers = get_keys_down();
    if (controllers.c[0].up) {
        if (--selected_game_index < 0) {
            selected_game_index = 0;
        }
    }

    if (controllers.c[0].down) {
        if (++selected_game_index >= romFileCount) {
            selected_game_index = romFileCount - 1;
        }
    }

    if (controllers.c[0].A) {
        on_game_selected(cpu_state, romFiles[selected_game_index]);
    }
}

void execute_game(C8_State *cpu_state, struct controller_data controllers) {
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
    update_button_states(cpu_state, key_map, controllers);

    // Start menu
    controllers = get_keys_down();
    if (controllers.c[0].start) {
        state = GAME_SELECT;
    }
}

int main(void) {
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
    console_init();

    struct controller_data controllers;
    controller_init();

    debug_init_usblog();
//    console_set_debug(true);

    fprintf(stdout, "Hello N64!\n");

    if (dfs_init(DFS_DEFAULT_LOCATION) != DFS_ESUCCESS) {
        printf("Filesystem failed to start!\n");
        return 1;
    }

    // Init C8
    C8_State cpu_state;
    printf("Initializing C8\n");
    C8_init(&cpu_state);

    while (1) {
        controller_scan();

        switch (state) {
            case GAME_SELECT:
                execute_game_select(&cpu_state);
                break;
            case CONTROLLER_SETUP:
                execute_controller_config();
                break;
            case GAME:
                execute_game(&cpu_state, controllers);
                break;
            default:
                printf("Unsupported state: %d\n", state);
                return 1;
        }
    }
}