/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 7. 4. 2019
 * Filename: adaptive_huffman.cpp
 * Description: Class declaration with function prototypes
 */

#ifndef ADAPTIVE_HUFFMAN_H
#define ADAPTIVE_HUFFMAN_H

#include <stdio.h>

#include <algorithm>
#include <queue>

#include "bitpacker.h"
#include "constants.h"
#include "tree_node.h"

class adaptive_huffman {
  tree_node *root;
  tree_node *symbols[SYMBOLS_ARRAY_SIZE];
  tree_node *orders[SYMBOLS_ARRAY_SIZE];
  byte_t *bytes;
  size_t len;
  std::deque<bool> symbol_bits;

public:
  adaptive_huffman(byte_t *bytes, size_t len);
  tree_node *create_subtree(byte_t symbol);
  void update_tree(tree_node *node);
  void encode_symbol(tree_node *node, bitpacker &bp);
  void encode(FILE *output);
  void decode(std::vector<byte_t> &decoded_bytes);
  void recursively_free(tree_node *node);
  ~adaptive_huffman();
};

#endif