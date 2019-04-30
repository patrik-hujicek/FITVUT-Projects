/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 4. 4. 2019
 * Filename: pixel_diff_model.h
 * Description: Function prototypes
 */

#ifndef PIXEL_DIFF_MODEL_H
#define PIXEL_DIFF_MODEL_H

#include "constants.h"

void apply_pixel_diff_model(byte_t *bytes, size_t len);
void reverse_pixel_diff_model(byte_t *bytes, size_t len);

#endif