/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 4. 4. 2019
 * Filename: static_huffman.cpp
 * Description: Implementation of static (canonical) huffman coding
 */

#include "static_huffman.h"

/*
 * brief Constructor
 * \param bytes input bytes
 * \param len length of input bytes
 */
static_huffman::static_huffman(byte_t *bytes, size_t len)
    : bytes{bytes}, len{len} {
  frequencies.resize(TABLE_SIZE);
  // Compute frequencies of alphabet symbols
  for (size_t i = 0; i < len; ++i)
    ++frequencies[bytes[i]];
};

/*
 * \brief Check if there is a next bit in the buffer
 * \return result of the check
 */
bool static_huffman::has_next_bit() {
  bool reached_end = false;
  if (padding != 0) {
    reached_end = (bytes_cnt + 1 == len) && (pos_in_byte == CHAR_BIT - padding);
  } else {
    reached_end = (bytes_cnt == len);
  }

  return !reached_end;
}

/*
 * \brief Get a next bit in the buffer
 * \return next bit
 */
bool static_huffman::get_next_bit() {
  byte_t current_byte = bytes[bytes_cnt];
  bool bit = current_byte & (1 << (CHAR_BIT - pos_in_byte - 1));
  if (pos_in_byte + 1 == CHAR_BIT) {
    bytes_cnt++;
    pos_in_byte = 0;
  } else {
    pos_in_byte++;
  }
  return bit;
}

/*
 * \brief Decode and store input bytes, implements FCFS and FastCHD methods
 * \param decoded_bytes decoded bytes
 */
void static_huffman::decode(std::vector<byte_t> &decoded_bytes) {

  size_t i = 0;
  size_t n = bytes[i];
  if (n + 1 >= len) {
    // Garbage, bail out
    return;
  }
  i++;
  size_t zero_lenghts_count = bytes[i];
  i++;
  padding = (zero_lenghts_count & BYTE_PADDING_MASK) >> BYTE_PADDING_OFFSET;
  pos_in_byte = 0;
  zero_lenghts_count = zero_lenghts_count & ~BYTE_PADDING_MASK;
  n -= zero_lenghts_count;
  std::vector<size_t> lengths;
  std::vector<size_t> first_code;
  std::vector<size_t> first_symbol;
  std::vector<byte_t> alphabet;

  for (size_t j = 0; j < zero_lenghts_count; ++j)
    lengths.emplace_back(0);

  size_t alphabet_len = 0;
  for (size_t e = 1 + n; i < e; ++i) {
    size_t len_i = bytes[i];
    // Detect situation when L_8 has 256 chars
    if (len_i == 0 && zero_lenghts_count == CHAR_BIT - 1)
      len_i = 256;
    lengths.emplace_back(len_i);
    alphabet_len += len_i;
  }

  if (n + 1 + alphabet_len >= len) {
    // Garbage, bail out
    return;
  }

  for (size_t e = n + 1 + alphabet_len; i < e; ++i)
    alphabet.emplace_back(bytes[i]);

  bytes_cnt = 1 + n + alphabet_len;

  {
    /* FCFS */
    size_t c = 0;
    size_t s = 1;
    first_code.emplace_back(c);
    first_symbol.emplace_back(s);
    for (size_t i = 0, e = lengths.size(); i < e; ++i) {
      s = s + lengths[i];
      c = (c + (lengths[i] - 1) + 1) << 1;

      first_code.emplace_back(c);
      first_symbol.emplace_back(s);
    }
  }

  {
    /* FastCHD */
    size_t c = 0;
    size_t l = 0;

    while (has_next_bit()) {
      bool b = get_next_bit();
      l = l + 1;
      c = (c << 1) + b;
      if ((c << 1) < first_code[l]) {
        size_t index = first_symbol[l - 1] + c - first_code[l - 1] - 1;
        if (index >= alphabet_len) {
          // Garbage, bail out
          return;
        }
        byte_t s = alphabet[index];
        decoded_bytes.emplace_back(s);
        l = 0;
        c = 0;
      }
    }
  }
}

/*
 * \brief Recursively save codes for all symbols
 * \param node current node
 * \param code symbol bits
 */
void static_huffman::save_codes(tree_node *node, std::vector<bool> code) {
  if (!node)
    return;
  if (node->is_leaf_node()) {
    codes.emplace_back(std::make_pair(node->get_symbol(), code));
    return;
  }

  std::vector<bool> l = code;
  std::vector<bool> r = code;
  l.emplace_back(0);
  r.emplace_back(1);
  save_codes(node->get_left_node(), l);
  save_codes(node->get_right_node(), r);
}

/*
 * \brief Recursively save codes for all symbols
 */
void static_huffman::save_codes() { save_codes(huffman_tree.top()); }

/*
 * \brief Build static huffman tree
 */
void static_huffman::build() {
  for (size_t i = 0; i < TABLE_SIZE; ++i) {
    size_t freq = frequencies[i];
    if (freq > 0)
      huffman_tree.push(new tree_node(i, freq));
  }

  // Add extra node to the tree when we have just one node
  if (huffman_tree.size() == 1) {
    huffman_tree.push(new tree_node(EXTRA, 1));
  }

  // Huffman algorithm
  while (huffman_tree.size() != 1) {
    tree_node *left = huffman_tree.top();
    huffman_tree.pop();

    tree_node *right = huffman_tree.top();
    huffman_tree.pop();

    // Create new internal node, add it to the tree
    tree_node *top =
        new tree_node(left->get_weight() + right->get_weight(), left, right);

    huffman_tree.push(top);
  }
}

/*
 * \brief Create canonical huffman tree
 */
void static_huffman::canonicalize() {
  // Sort by code lengths
  std::sort(codes.begin(), codes.end(),
            [](const std::pair<byte_t, std::vector<bool>> &lhs,
               const std::pair<byte_t, std::vector<bool>> &rhs) {
              return lhs.second.size() < rhs.second.size();
            });

  // Canonicalization
  // First
  std::pair<byte_t, std::vector<bool>> &s_1 = codes[0];
  size_t l_1 = s_1.second.size();
  size_t c_1 = 0;
  s_1.second.clear();
  for (size_t i = 0; i < l_1; ++i)
    s_1.second.emplace_back(0);

  if (codes.size() > 1) {
    // Second
    std::pair<byte_t, std::vector<bool>> &s_2 = codes[1];
    size_t l_2 = s_2.second.size();
    size_t c_2 = (c_1 + 1) << (l_2 - l_1);
    size_t l_i_prev = l_2;
    size_t c_i_prev = c_2;

    s_2.second.clear();
    while (c_2) {
      bool set = c_2 & 1 ? 1 : 0;
      s_2.second.emplace_back(set);
      c_2 >>= 1;
    }

    while (s_2.second.size() < l_2) {
      s_2.second.emplace_back(0);
    }

    std::reverse(s_2.second.begin(), s_2.second.end());

    // Rest
    for (size_t i = 2, e = codes.size(); i < e; ++i) {
      std::pair<byte_t, std::vector<bool>> &s_i = codes[i];
      size_t l_i = s_i.second.size();
      size_t c_i = (c_i_prev + 1) * (1 << (l_i - l_i_prev));

      l_i_prev = l_i;
      c_i_prev = c_i;

      s_i.second.clear();
      while (c_i) {
        bool set = c_i & 1 ? 1 : 0;
        s_i.second.emplace_back(set);
        c_i >>= 1;
      }

      while (s_i.second.size() < l_i) {
        s_i.second.emplace_back(0);
      }

      std::reverse(s_i.second.begin(), s_i.second.end());
    }
  }
}

/*
 * \brief Encode all input bytes to the output
 * \param output output file
 */
void static_huffman::encode(FILE *output) {
  if (len == 0)
    return;

  // Build huffman tree, save codes, canonicalize codes
  build();
  save_codes();
  canonicalize();

  bitpacker bp;
  byte_t n = 0;
  for (auto &s_c : codes)
    n = std::max((byte_t)s_c.second.size(), n);

  // How many zero lengths we have
  byte_t zero_lenghts_count = n;
  for (auto &s_c : codes)
    zero_lenghts_count =
        std::min((byte_t)s_c.second.size(), zero_lenghts_count);

  zero_lenghts_count--;
  n++;
  std::vector<std::vector<byte_t>> header;
  header.resize(n);
  std::vector<std::vector<bool>> table;
  table.resize(TABLE_SIZE);

  for (auto &s_c : codes) {
    byte_t li = s_c.second.size();
    table[s_c.first] = s_c.second;
    header[li].emplace_back(s_c.first);
  }

  // Encode symbols
  for (size_t i = 0; i < len; ++i)
    bp.add_bits(table[bytes[i]]);

  // Store last byte padding in the high tree bits of the offset of non zero
  // lenghts
  unsigned padding = bp.get_remainder_bits_count();
  size_t zero_lenghts_count_padding = zero_lenghts_count;
  zero_lenghts_count_padding |= padding << BYTE_PADDING_OFFSET;
  fputc(n, output);
  fputc(zero_lenghts_count_padding, output);

  // Write lengths
  for (int i = zero_lenghts_count + 1, e = header.size(); i < e; ++i) {
    fputc((byte_t)header[i].size(), output);
  }

  // Write alphabet
  for (auto &i : header)
    for (byte_t c : i)
      fputc(c, output);

  // Write encoded symbols
  for (auto &byte : bp.get_bytes())
    fputc(byte, output);
}

/*
 * \brief Recursively free nodes in the tree
 * \param node starting node
 */
void static_huffman::recursively_free(tree_node *node) {
  if (node == nullptr)
    return;

  recursively_free(node->get_left_node());
  recursively_free(node->get_right_node());
  delete node;
}

/*
 * \brief Destructor frees the tree
 */
static_huffman::~static_huffman() {
  while (!huffman_tree.empty()) {
    tree_node *node = huffman_tree.top();
    huffman_tree.pop();
    recursively_free(node);
  }
}