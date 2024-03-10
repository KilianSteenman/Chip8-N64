#include <stdio.h>
#include <libdragon.h>

#include "file_utils.h

int main(void) {
    console_init();

    debug_init_usblog();
    console_set_debug(true);

    printf("Hello N64!\n");

    // Load Rom (TODO: Add rom selection screen)
    FILE *rom;
    char *buffer;
    long fileLength;

    rom = fopen("rom://1-chip8-logo.ch8", "r");
    if (rom == NULL) {
        printf("Unable to open game rom\n");
        return;
    }


    fread(game_id, 3, 1, game_rom);
    fclose(game_rom);

    // Init C8
    C8_CPU_State cpu_state;
    C8_init(&cpu_state);
    C8_load_program(&cpu_state, buffer, fileLength);

    while (1) {}
}