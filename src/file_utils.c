//
// Created by Shadow-Link on 10/03/2024.
//

#include <stdio.h>
#include "file_utils.h"

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
