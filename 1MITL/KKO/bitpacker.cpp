/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 30. 3. 2019
 * Filename: bitpacker.h
 * Description: Implementation of bit buffer
 */

#include "bitpacker.h"

/*
 * Constructor
 */
bitpacker::bitpacker() : bytes{0}, pos_in_byte{0} {}

/*
 * \brief Add bit to bit buffer
 * \param bit bit to add
 */
void bitpacker::add_bit(bool bit) {
  if (pos_in_byte < CHAR_BIT) {
    bytes.back() |= bit << (CHAR_BIT - pos_in_byte - 1);
    pos_in_byte++;
  } else {
    bytes.emplace_back(bit << (CHAR_BIT - 1));
    pos_in_byte = 1;
  }
}

/*
 * \brief Add bits of byte to bit buffer
 * \param byte bits of byte to add
 */
void bitpacker::add_byte(byte_t c) {
  for (size_t i = 0; i < CHAR_BIT; ++i)
    add_bit((c >> (CHAR_BIT - i - 1)) & 1);
}

/*
 * \brief Add bits to bit buffer
 * \param bits bits to add
 */
void bitpacker::add_bits(std::vector<bool> &bits) {
  for (bool b : bits)
    add_bit(b);
}

/*
 * \brief Get number of remaining bits to the whole byte
 * \return remaining number of bits to the whole byte
 */
unsigned bitpacker::get_remainder_bits_count() {
  return (CHAR_BIT - pos_in_byte) % CHAR_BIT;
}

/*
 * \brief Get all stored bytes
 * \return stored bytes
 */
std::vector<byte_t> &bitpacker::get_bytes() { return bytes; }