//
// Created by Shadow-Link on 11/04/2024.
//

#ifndef CHIP_8_SHA1_H
#define CHIP_8_SHA1_H

#include <stdint.h>

int sha1digest(uint8_t *digest, char *hexdigest, const uint8_t *data, int databytes);

#endif //CHIP_8_SHA1_H
