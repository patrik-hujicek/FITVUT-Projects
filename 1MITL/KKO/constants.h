#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>
#include <stdlib.h>

enum { invalid, encoding, decoding };

typedef unsigned char byte_t;
const uint16_t TABLE_SIZE = 256;
const uint16_t SYMBOLS_SIZE = 257;
const uint16_t SYMBOLS_ARRAY_SIZE = 2 * SYMBOLS_SIZE;
const byte_t BYTE_PADDING_OFFSET = 5;
const byte_t BYTE_PADDING_MASK = 0b11100000;

#endif