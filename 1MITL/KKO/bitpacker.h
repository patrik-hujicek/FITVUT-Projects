/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 30. 3. 2019
 * Filename: bitpacker.h
 * Description: Class declaration with function prototypes
 */

#ifndef BITPACKER_H
#define BITPACKER_H

#include <limits.h>
#include <vector>

#include "constants.h"

class bitpacker {
  std::vector<byte_t> bytes;
  size_t pos_in_byte;

public:
  bitpacker();
  void add_bit(bool bit);
  void add_byte(byte_t c);
  void add_bits(std::vector<bool> &bits);
  unsigned get_remainder_bits_count();
  std::vector<byte_t> &get_bytes();
};

#endif