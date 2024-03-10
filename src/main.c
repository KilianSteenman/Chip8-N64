#include <stdio.h>
#include <libdragon.h>

#include "file_utils.h"
#include "chip8_cpu.h"

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
        return 1;
    }

    // Get the size of the rom
    fileLength = getFileSize(rom);

    // Allocate memory for the buffer to store file contents
    buffer = (char *) malloc(fileLength);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(rom);
        return 1;
    }

    // Read file contents into the buffer
    fread(buffer, 1, fileLength, rom);

    // Cleanup
    fclose(rom);

    // Init C8
    C8_CPU_State cpu_state;
    C8_init(&cpu_state);
    C8_load_program(&cpu_state, buffer, fileLength);

    while (1) {
        C8_execute_program(&cpu_state);
    }
}