/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 7. 4. 2019
 * Filename: adaptive_huffman.cpp
 * Description: Implementation of adaptive huffman coding - algorithm FGK
 */

#include "adaptive_huffman.h"
/*
 * brief Constructor
 * \param bytes input bytes
 * \param len length of input bytes
 */
adaptive_huffman::adaptive_huffman(byte_t *bytes, size_t len)
    : symbols{}, orders{}, bytes{bytes}, len{len} {
  // At start, put NYT node at the tree
  root = new tree_node();
  root->set_symbol(NYT);
  root->set_order(SYMBOLS_ARRAY_SIZE);
  symbols[NYT] = root;
}

/*
 * \brief Create subtree for the symbol's node
 * \param symbol symbol
 */
tree_node *adaptive_huffman::create_subtree(byte_t symbol) {
  // Create two children
  tree_node *nyt_node = symbols[NYT];
  tree_node *left_child = new tree_node();
  tree_node *right_child = new tree_node();
  nyt_node->set_left_node(left_child);
  nyt_node->set_right_node(right_child);
  nyt_node->set_symbol(INTERNAL);

  uint16_t nyt_order = nyt_node->get_order();

  // Set nyt_node as a parent of children
  // Adjust their order to keep correct ordering
  left_child->set_parent_node(nyt_node);
  left_child->set_order(nyt_order - 2);
  left_child->set_symbol(NYT);

  right_child->set_parent_node(nyt_node);
  right_child->set_order(nyt_order - 1);
  right_child->set_symbol(symbol);
  orders[nyt_order - 1] = right_child;

  symbols[NYT] = left_child;
  symbols[symbol] = right_child;
  symbols[SYMBOLS_ARRAY_SIZE - (nyt_order / 2)] = nyt_node;
  return right_child;
}

void adaptive_huffman::update_tree(tree_node *node) {
  tree_node *node_parent = node->get_parent_node();
  if (node_parent == root) {
    node->increment_weight();
    root->increment_weight();
    return;
  }

  uint16_t node_order = node->get_order();
  size_t node_weight = node->get_weight();
  tree_node *highest_node = node_parent;
  uint16_t highest_order;

  if (node_parent == symbols[NYT]->get_parent_node()) {
    // Find highest (by order) node in the node's block
    for (tree_node **p = orders + node_order + 1,
                   **end = orders + SYMBOLS_ARRAY_SIZE;
         p < end; ++p) {
      tree_node *symbol_node = *p;
      if (symbol_node && symbol_node->get_symbol() < TABLE_SIZE) {
        if (symbol_node->get_weight() != node_weight)
          break;

        highest_node = symbol_node;
      }
    }
    goto swap_nodes;
  } else {
    goto find_highest_hode;
  }

  do {
    node_parent = node->get_parent_node();

    if (node_parent == root) {
      node->increment_weight();
      root->increment_weight();
      return;
    }

    node_order = node->get_order();
    node_weight = node->get_weight();
    highest_node = node_parent;

  find_highest_hode:
    // Find highest (by order) node in the node's block
    for (tree_node **p = orders + node_order + 1,
                   **end = orders + SYMBOLS_ARRAY_SIZE;
         p < end; ++p) {
      tree_node *symbol_node = *p;
      if (symbol_node) {
        if (symbol_node->get_weight() != node_weight)
          break;

        highest_node = symbol_node;
      }
    }
  swap_nodes:
    // Increment weight
    node->increment_weight();
    if (__builtin_expect(highest_node != node_parent, 0)) {
      // Swap node with the highest node
      if (node_parent->get_right_node() == node)
        node_parent->set_right_node(highest_node);
      else
        node_parent->set_left_node(highest_node);

      tree_node *highest_node_parent = highest_node->get_parent_node();
      if (highest_node_parent->get_right_node() == highest_node)
        highest_node_parent->set_right_node(node);
      else
        highest_node_parent->set_left_node(node);

      highest_order = highest_node->get_order();
      node->set_parent_node(highest_node_parent);
      node->set_order(highest_order);
      orders[highest_order] = node;
      highest_node->set_parent_node(node_parent);
      highest_node->set_order(node_order);
      orders[node_order] = highest_node;
      node = highest_node_parent;
    } else {
      node = node_parent;
    }
  } while (node != root);
  node->increment_weight();
}

/*
 * \brief Get symbol's code and add it to the buffer of bits
 * \param node symbol's node
 * \param bp buffer of bits
 */
void adaptive_huffman::encode_symbol(tree_node *node, bitpacker &bp) {
  tree_node *node_parent;
  bool b;
  do {
    node_parent = node->get_parent_node();
    b = node == node_parent->get_right_node();
    symbol_bits.emplace_front(b);
    node = node_parent;
  } while (node != root);

  // Add symbol bits to bit buffer
  for (bool b : symbol_bits)
    bp.add_bit(b);

  // Clear current symbol bits
  symbol_bits.clear();
}

/*
 * \brief Encode all input bytes to the output
 * \param output output file
 */
void adaptive_huffman::encode(FILE *output) {
  if (len == 0)
    return;
  bitpacker bp;
  // First byte
  byte_t s = bytes[0];
  bp.add_byte(s);
  update_tree(create_subtree(s));

  // Other bytes
  for (size_t i = 1; i < len; ++i) {
    byte_t s = bytes[i];
    tree_node *sym_node = symbols[s];
    if (!sym_node) {
      encode_symbol(symbols[NYT], bp);
      update_tree(create_subtree(s));
      bp.add_byte(s);
    } else {
      encode_symbol(sym_node, bp);
      update_tree(sym_node);
    }
  }

  std::vector<byte_t> &bytes = bp.get_bytes();
  for (byte_t s : bytes)
    fputc(s, output);

  // No padding and last byte is higher than highest possible padding (7)
  // We can bail out here, a decoder will be able to decode this file
  if (bp.get_remainder_bits_count() == 0 && bytes.back() > CHAR_BIT - 1) {
    return;
  }

  // Write padding info
  fputc(bp.get_remainder_bits_count(), output);
}

/*
 * \brief Decode and store input bytes
 * \param decoded_bytes decoded bytes
 */
void adaptive_huffman::decode(std::vector<byte_t> &decoded_bytes) {
  tree_node *node = root;
  bool seen_NYT = true;
  byte_t buffer = 0;
  unsigned padding = bytes[len - 1];
  // If padding is more than 7, we have no padding actually
  size_t bits = (padding > CHAR_BIT - 1) ? (len * CHAR_BIT)
                                         : ((len - 1) * CHAR_BIT) - padding;
  size_t bits_in_byte = 0;
  byte_t *p = bytes;
  while (true) {
    byte_t c = *p++;
    for (size_t j = 0; j < CHAR_BIT; ++j) {
      // Current bit
      bool b = c & (1 << (CHAR_BIT - j - 1));
      if (__builtin_expect(!seen_NYT, 1)) {
        // Tree traversal
        tree_node *right = node->get_right_node();
        tree_node *left = node->get_left_node();
        node = b ? right : left;
        if (!node) {
          // Garbage, bail out
          return;
        }
        uint16_t symbol = node->get_symbol();
        if (__builtin_expect(symbol <= NYT, 0)) {
          if (__builtin_expect(symbol == NYT, 0)) {
            seen_NYT = true;
            node = root;
          } else {
            // Get the symbol of the node
            byte_t s = symbol;
            decoded_bytes.emplace_back(s);
            // Add the symbol to the tree, update the tree
            update_tree(node);
            node = root;
          }
        }
      } else {
        // Seen NYT
        buffer |= (b << (CHAR_BIT - bits_in_byte - 1));
        bits_in_byte++;

        if (bits_in_byte == CHAR_BIT) {
          // We have 8 bits (byte)
          byte_t s = buffer;
          decoded_bytes.emplace_back(s);
          // Add the symbol to the tree, update the tree
          update_tree(create_subtree(s));
          seen_NYT = false;
          bits_in_byte = 0;
          buffer = 0;
        }
      }

      --bits;
      if (bits == 0) {
        // All bits processed, finish
        return;
      }
    }
  }
}

/*
 * \brief Recursively free nodes in the tree
 * \param node starting node
 */
void adaptive_huffman::recursively_free(tree_node *node) {
  if (node == nullptr)
    return;

  recursively_free(node->get_left_node());
  recursively_free(node->get_right_node());
  delete node;
}

/*
 * \brief Destructor frees the tree
 */
adaptive_huffman::~adaptive_huffman() { recursively_free(root); }