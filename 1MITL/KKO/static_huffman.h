/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 4. 4. 2019
 * Filename: static_huffman.h
 * Description: Class declaration with function prototypes
 */

#ifndef STATIC_HUFFMAN_H
#define STATIC_HUFFMAN_H

#include <stdio.h>

#include <algorithm>
#include <queue>

#include "bitpacker.h"
#include "constants.h"
#include "tree_node.h"

class static_huffman {
  std::vector<size_t> frequencies;
  std::vector<std::pair<byte_t, std::vector<bool>>> codes;
  std::priority_queue<tree_node *, std::vector<tree_node *>, tree_node_compare>
      huffman_tree;
  byte_t *bytes;
  size_t len;
  size_t bytes_cnt;
  size_t pos_in_byte;
  unsigned padding;

public:
  static_huffman(byte_t *bytes, size_t len);
  bool has_next_bit();
  bool get_next_bit();
  void fcfs();
  void decode(std::vector<byte_t> &decoded_bytes);
  void save_codes(tree_node *root, std::vector<bool> code = {});
  void save_codes();
  void build();
  void canonicalize();
  void encode(FILE *output);
  void recursively_free(tree_node *node);
  ~static_huffman();
};

#endif