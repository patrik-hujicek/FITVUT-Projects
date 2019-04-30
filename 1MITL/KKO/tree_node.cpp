/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 8. 3. 2019
 * Filename: tree_node.cpp
 * Description: Implementation of a tree node
 */

#include "tree_node.h"

/* Constructors */
tree_node::tree_node()
    : weight{0}, parent{nullptr}, left{nullptr}, right{nullptr}, symbol{0},
      order{0} {}
tree_node::tree_node(uint16_t symbol, size_t weight)
    : weight{weight}, parent{nullptr}, left{nullptr}, right{nullptr},
      symbol{symbol} {}
tree_node::tree_node(size_t weight, tree_node *left, tree_node *right)
    : weight{weight}, parent{nullptr}, left{left}, right{right} {}

/* Getter and setters */
uint16_t tree_node::get_symbol() { return symbol; }
size_t tree_node::get_weight() { return weight; }
uint16_t tree_node::get_order() { return order; }
void tree_node::set_order(uint16_t new_order) { order = new_order; }
void tree_node::increment_weight() { ++weight; }
void tree_node::set_weight(size_t new_weight) { weight = new_weight; }
void tree_node::set_symbol(uint16_t new_symbol) { symbol = new_symbol; }
void tree_node::set_left_node(tree_node *new_left) { left = new_left; }
void tree_node::set_right_node(tree_node *new_right) { right = new_right; }
void tree_node::set_parent_node(tree_node *new_parent) { parent = new_parent; }
tree_node *tree_node::get_left_node() { return left; }
tree_node *tree_node::get_right_node() { return right; }
tree_node *tree_node::get_parent_node() { return parent; }
/* Node type checkers */
bool tree_node::is_leaf_node() { return left == nullptr && right == nullptr; }
/* Compare nodes by their weights */
bool tree_node::operator<(tree_node &other) const {
  return weight < other.weight;
}

/*
 * \brief Node comparator according to the weights of nodes
 */
bool tree_node_compare::operator()(tree_node *left, tree_node *right) {
  return (left->get_weight() > right->get_weight());
}
