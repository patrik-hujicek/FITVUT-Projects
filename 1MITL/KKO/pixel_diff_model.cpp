/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 4. 4. 2019
 * Filename: pixel_diff_model.cpp
 * Description: Implementation of the pixel difference model
 */

#include "pixel_diff_model.h"

/*
 * \brief Convert the bytes using the pixel difference model
 * \param bytes bytes array
 * \param len length of the bytes array
 */
void apply_pixel_diff_model(byte_t *bytes, size_t len) {
  byte_t prev = bytes[0];
  for (size_t i = 1; i < len; ++i) {
    byte_t current = bytes[i];
    bytes[i] = current - prev;
    prev = current;
  }
}

/*
 * \brief Convert bytes using the reverse pixel difference model
 * \param bytes bytes array
 * \param len length of the bytes array
 */
void reverse_pixel_diff_model(byte_t *bytes, size_t len) {
  for (size_t i = 1; i < len; ++i) {
    byte_t current = bytes[i];
    bytes[i] = current + bytes[i - 1];
  }
}