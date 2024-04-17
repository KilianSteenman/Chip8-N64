#include <stdio.h>
#include <string.h>
#include <libdragon.h>

#include "rom.h"
#include "file_utils.h"
#include "chip8.h"
#include "input.h"
#include "screen_controller_config.h"
#include "screen_rom_select.h"
#include "screen_rom_execution.h"
#include "sha1.h"

typedef enum {
    ROM_SELECT,
    CONTROLLER_SETUP,
    EXECUTE_ROM,
} State;

State state = ROM_SELECT;
char *selected_rom_name;

KeyMap key_map;

char romFiles[9][30] = {
        "rom://1-chip8-logo.ch8",
        "rom://2-ibm-logo.ch8",
        "rom://3-corax+.ch8",
        "rom://4-flags.ch8",
        "rom://6-keypad.ch8",
        "rom://BC_test.ch8",
        "rom://Tetris.ch8",
        "rom://SpaceInvaders.ch8",
        "rom://Pong.ch8"
};

int selected_game_index = 0;

Rom *load_rom(char *name) {
    console_set_render_mode(RENDER_AUTOMATIC);
    console_clear();

    FILE *romFile;
    uint8_t *buffer;
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
    buffer = (uint8_t *) malloc(fileLength);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(romFile);
        return NULL;
    }
    printf("Allocated memory %ld\n", fileLength);

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

    // Load rom
    Rom *rom = load_rom(romFile);
    if (rom == NULL) {
        printf("Unable to load rom!\n");
        return;
    }

    // Try to load config
    uint8_t hash[20];
    char hexHash[41];
    sha1digest(hash, hexHash, rom->buffer, rom->fileLength);
    printf("Sha1: %s\n", hexHash);
    char configFileName[52];
    sprintf(configFileName, "rom://%s.c864", hexHash);
    FILE *configFile = fopen(configFileName, "r");
    if (configFile == NULL) {
        printf("No config found for %s\n", hexHash);
        init_key_map(&key_map);
        state = CONTROLLER_SETUP;
        return;
    }

    // Load the controller config
    printf("Loading config\n");
    init_key_map(&key_map);
    uint8_t *configBuffer;
    long fileLength;

    // Get the size of the config file
    fileLength = getFileSize(configFile);

    // Allocate memory for the buffer to store file contents
    configBuffer = (uint8_t *) malloc(fileLength);
    if (configBuffer == NULL) {
        perror("Memory allocation failed");
        fclose(configFile);
        return;
    }
    fread(configBuffer, 1, fileLength, configFile);
    fclose(configFile);

    printf("Loading key map\n");
    load_key_map(configBuffer, &key_map);
    free(configBuffer);

    printf("KeyMap:\n");
    for(int i = 0; i < 16; i++) {
        printf("%02x ", key_map.key[i]);
    }

    C8_init(cpu_state);
    C8_load_program(cpu_state, rom);
    free(rom);
    state = EXECUTE_ROM;
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

    // Init keymap
    init_key_map(&key_map);

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
                    store_key_map(selected_rom_name, &key_map);
                    state = EXECUTE_ROM;
                }
                break;
            case EXECUTE_ROM:
                if (execute_rom(&cpu_state, controllers, &key_map)) {
                    state = ROM_SELECT;
                }
                break;
            default:
                printf("Unsupported state: %d\n", state);
                return 1;
        }
    }
}