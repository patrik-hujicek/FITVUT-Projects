/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 8. 3. 2019
 * Filename: tree_node.h
 * Description: Class declaration with function prototypes
 */

#ifndef TREE_NODE_H
#define TREE_NODE_H

#include "constants.h"

// Special code for NYT node
const uint16_t NYT = 256;
// Special code for internal node
const uint16_t INTERNAL = 257;
// Special code for extra node
const uint16_t EXTRA = 256;

class tree_node {
  size_t weight;
  tree_node *parent;
  tree_node *left;
  tree_node *right;
  uint16_t symbol;
  uint16_t order;

public:
  tree_node();
  tree_node(uint16_t symbol, size_t weight);
  tree_node(size_t weight, tree_node *left, tree_node *right);

  uint16_t get_symbol();
  size_t get_weight();
  uint16_t get_order();
  void set_symbol(uint16_t new_symbol);
  void increment_weight();
  void set_weight(size_t new_weight);
  void set_order(uint16_t order);
  void set_left_node(tree_node *new_left);
  void set_right_node(tree_node *new_right);
  void set_parent_node(tree_node *new_parent);
  tree_node *get_left_node();
  tree_node *get_right_node();
  tree_node *get_parent_node();
  bool is_leaf_node();
  bool is_internal_node();
  bool operator<(tree_node &other) const;
};

struct tree_node_compare {
  bool operator()(tree_node *left, tree_node *right);
};

#endif