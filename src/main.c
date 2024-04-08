#include <stdio.h>
#include <string.h>
#include <libdragon.h>

#include "rom.h"
#include "file_utils.h"
#include "chip8.h"
#include "input.h"
#include "screen_controller_config.h"
#include "screen_rom_select.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define GRID_SIZE 1 // Size of each grid cell
#define GRID_SCALE 1

typedef enum {
    ROM_SELECT,
    CONTROLLER_SETUP,
    EXECUTE_ROM,
} State;

State state = ROM_SELECT;
char *selected_rom_name;

KeyMap key_map;

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

void store_binding(char *name, KeyMap *key_map) {
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

void load_binding(uint8_t *buffer, KeyMap *key_map) {
    // Use the second 16 bytes to load the keymap
    for (int i = 0; i < 16; i++) {
        key_map->key[i] = buffer[i + 16];
    }
}

void load_controller_config(char *name) {
    // When running on a flash cart we should probably save to a file
    // For now we save to eeprom

    // Create and allocate memory for an 32-byte buffer
    uint8_t *buffer = malloc(32 * sizeof(uint8_t));

    // Load the data from block 0 to the buffer
    eeprom_read_bytes(buffer, 0, 32);
    printf("\n");
    for (int i = 0; i < 32; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n");
    bool matches = true;
    for (int i = 0; i < 16; i++) {
        if (buffer[i] != name[i]) {
            matches = false;
        }
    }
    if (matches) {
        printf("Matches\n");
        load_binding(buffer, &key_map);
        state = EXECUTE_ROM;
    } else {
        printf("No match\n");
        state = CONTROLLER_SETUP;
        init_key_map(&key_map);
        selected_rom_name = name;
    }

    free(buffer);
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
    printf("Allocated rom\nFile memory %ld\n", fileLength);

    // Read file contents into the buffer
    fread(buffer, 1, fileLength, romFile);

    // Cleanup
    fclose(romFile);

    Rom *rom = malloc(sizeof(Rom));
    rom->buffer = buffer;
    rom->fileLength = fileLength;
    return rom;
}

void on_rom_selected(C8_State *cpu_state, char *romFile) {
    console_clear();
    console_set_render_mode(RENDER_AUTOMATIC);

    selected_rom_name = romFile;
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

void execute_rom(C8_State *cpu_state, struct controller_data controllers) {
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
        state = ROM_SELECT;
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

    eeprom_type_t type = eeprom_present();
    size_t size = eeprom_total_blocks();
    printf("eeprom %d size %d\n", type, size);

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
            case ROM_SELECT:
                int rom_count = sizeof(romFiles) / sizeof(romFiles[0]);
                if (execute_rom_select(romFiles, rom_count, &selected_game_index)) {
                    on_rom_selected(&cpu_state, romFiles[selected_game_index]);
                }
                break;
            case CONTROLLER_SETUP:
                if (execute_controller_config(&key_map)) {
                    store_binding(selected_rom_name, &key_map);
                    state = EXECUTE_ROM;
                }
                break;
            case EXECUTE_ROM:
                execute_rom(&cpu_state, controllers);
                break;
            default:
                printf("Unsupported state: %d\n", state);
                return 1;
        }
    }
}