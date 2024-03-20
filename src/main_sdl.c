#include <SDL2/SDL.h>

#include "file_utils.h"
#include "chip8.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define GRID_SIZE 1 // Size of each grid cell
#define GRID_SCALE 10

uint8_t display[SCREEN_WIDTH * SCREEN_HEIGHT * 4];

void renderGrid(C8_CPU_State *state, SDL_Renderer *renderer) {
    for (int x = 0; x < SCREEN_WIDTH; x += GRID_SIZE) {
        for (int y = 0; y < SCREEN_HEIGHT; y += GRID_SIZE) {

            int xOffset = x / GRID_SCALE;
            int yOffset = y / GRID_SCALE;

            int pixelIndex = (y * SCREEN_WIDTH + x) * 4;
            if (state->display[yOffset][xOffset]) {
                display[pixelIndex] = 0;
                display[pixelIndex + 1] = 0;
                display[pixelIndex + 2] = 0;
                display[pixelIndex + 3] = 255;
            } else {
                display[pixelIndex] = 255;
                display[pixelIndex + 1] = 255;
                display[pixelIndex + 2] = 255;
                display[pixelIndex + 3] = 255;
            }
        }
    }
}

int main() {
    FILE *file;
    char *buffer;
    long fileLength;

    // Open the file in binary read mode
//    file = fopen("test_rom.ch8", "rb");
//    file = fopen("2-ibm-logo.ch8", "rb");
//        file = fopen("3-corax+.ch8", "rb");
    file = fopen("4-flags.ch8", "rb");
//    file = fopen("ibm_logo.c8", "rb");
//    file = fopen("c8_test.c8", "rb");
//    file = fopen("test_opcode.ch8", "rb");
//    file = fopen("BC_test.ch8", "rb");
//    file = fopen("Tetris.ch8", "rb");
//    file = fopen("Pong.ch8", "rb");
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
    C8_init(&cpu_state);
    C8_load_program(&cpu_state, buffer, fileLength);

    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Chip 8");

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black
    SDL_RenderClear(renderer);

    SDL_Texture *texTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Event event;
    int quit = 0;
    int exec = 0;
    while (!quit) {
        C8_execute_program(&cpu_state);

        if (cpu_state.draw == 1) {
            renderGrid(&cpu_state, renderer); // Render grid
            SDL_UpdateTexture(texTarget, NULL, display, SCREEN_WIDTH * 4);

            SDL_RenderClear(renderer);
            SDL_RenderCopyEx(renderer, texTarget, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
            SDL_RenderPresent(renderer); // Update screen

            cpu_state.draw = 0;
        }

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
