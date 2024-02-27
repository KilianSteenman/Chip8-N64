#include <SDL2/SDL.h>

#include "chip8_cpu.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define GRID_SIZE 1 // Size of each grid cell

// Function to get the size of a file
long getFileSize(FILE *file) {
    long size;

    // Move the file cursor to the end of the file
    fseek(file, 0, SEEK_END);

    // Get the current position of the cursor (which is the size of the file)
    size = ftell(file);

    // Move the cursor back to the beginning of the file
    fseek(file, 0, SEEK_SET);

    return size;
}

void renderGrid(C8_CPU_State *state, SDL_Renderer *renderer) {
    for (int x = 0; x < SCREEN_WIDTH; x += GRID_SIZE) {
        for (int y = 0; y < SCREEN_HEIGHT; y += GRID_SIZE) {

            int xOffset = x / 10;
            int yOffset = y / 10;

            if (state->display[yOffset][xOffset]) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set color to black
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set color to white
            }
            SDL_Rect rect = {x, y, GRID_SIZE, GRID_SIZE};
            SDL_RenderFillRect(renderer, &rect); // Draw grid cell
        }
    }
}

char fontArray[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

int main() {
    FILE *file;
    char *buffer;
    long fileLength;

    // Open the file in binary read mode
//    file = fopen("1-chip8-logo.ch8", "rb");
//    file = fopen("2-ibm-logo.ch8", "rb");
        file = fopen("3-corax+.ch8", "rb");
//    file = fopen("4-flags.ch8", "rb");
//    file = fopen("ibm_logo.c8", "rb");
//    file = fopen("c8_test.c8", "rb");
//    file = fopen("test_opcode.ch8", "rb");
//    file = fopen("BC_test.ch8", "rb");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Get the size of the file
    fileLength = getFileSize(file);

    // Allocate memory for the buffer to store file contents
    buffer = (char *) malloc(fileLength);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    // Read file contents into the buffer
    fread(buffer, 1, fileLength, file);

    // Clean up: Close the file and free the buffer
    fclose(file);

    C8_CPU_State cpu_state;
    C8_clear_screen(&cpu_state);
    C8_load_font(&cpu_state, &fontArray, sizeof(fontArray));
    C8_load_program(&cpu_state, buffer, fileLength);

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Grid Renderer");

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black
    SDL_RenderClear(renderer);

    SDL_Event event;
    int quit = 0;
    while (!quit) {
        C8_execute_program(&cpu_state);

        renderGrid(&cpu_state, renderer); // Render grid

        SDL_RenderPresent(renderer); // Update screen

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
        }
    }

    free(buffer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
