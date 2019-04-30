#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <getopt.h>

#include <algorithm>
#include <bitset>
#include <fstream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "bitpacker.h"
#include "constants.h"
#include "pixel_diff_model.h"
#include "tree_node.h"

class decoder {
  byte_t *bytes;
  size_t len;
  std::vector<size_t> first_code;
  std::vector<size_t> first_symbol;
  std::vector<byte_t> alphabet;
  size_t bytes_cnt;
  size_t pos_in_byte;
  unsigned padding;

public:
  decoder(byte_t *bytes, size_t len) : bytes{bytes}, len{len} {}

  bool has_next_bit() {
    bool reached_end = false;
    if (padding == 0) {
      reached_end = (bytes_cnt == len);
    } else {
      reached_end =
          (bytes_cnt + 1 == len) && (pos_in_byte == CHAR_BIT - padding);
    }

    return !reached_end;
  }

  bool get_next_bit() {
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

  void fcfs() {
    size_t i = 0;
    byte_t n = bytes[i];
    i++;
    byte_t zero_lenghts_count = bytes[i];
    i++;
    padding = (zero_lenghts_count & BYTE_PADDING_MASK) >> BYTE_PADDING_OFFSET;
    pos_in_byte = 0;
    zero_lenghts_count = zero_lenghts_count & ~BYTE_PADDING_MASK;
    n -= zero_lenghts_count;
    std::vector<size_t> lengths;

    for (size_t j = 0; j < zero_lenghts_count; ++j)
      lengths.push_back(0);

    size_t alphabet_len = 0;
    for (size_t e = 1 + n; i < e; ++i) {
      byte_t len_i = bytes[i];
      lengths.push_back(len_i);
      alphabet_len += len_i;
    }

    for (size_t e = n + 1 + alphabet_len; i < e; ++i)
      alphabet.push_back(bytes[i]);

    bytes_cnt = 1 + n + alphabet_len;

    size_t c = 0;
    size_t s = 1;
    first_code.push_back(c);
    first_symbol.push_back(s);
    for (byte_t i = 0, e = lengths.size(); i < e; ++i) {
      s = s + lengths[i];
      c = (c + (lengths[i] - 1) + 1) << 1;

      first_code.push_back(c);
      first_symbol.push_back(s);
    }
  }

  void fastCHD(std::vector<byte_t> &decoded_bytes) {
    size_t c = 0;
    size_t l = 0;

    while (has_next_bit()) {
      bool b = get_next_bit();
      l = l + 1;
      c = (c << 1) + b;
      if ((c << 1) < first_code[l]) {
        byte_t s = alphabet[first_symbol[l - 1] + c - first_code[l - 1] - 1];
        decoded_bytes.push_back(s);
        l = 0;
        c = 0;
      }
    }
  }
};

class static_huffman {
  std::vector<size_t> frequencies;
  std::vector<std::pair<byte_t, std::vector<bool>>> codes;
  std::priority_queue<tree_node *, std::vector<tree_node *>, tree_node_compare>
      huffman_tree;
  byte_t *bytes;
  size_t len;

public:
  static_huffman(byte_t *bytes, size_t len) : bytes{bytes}, len{len} {
    frequencies.resize(TABLE_SIZE);
    for (size_t i = 0; i < len; ++i)
      frequencies[bytes[i]]++;
  };

  void save_codes(tree_node *root, std::vector<bool> code = {}) {
    if (!root)
      return;
    if (!root->is_internal_node()) {
      codes.emplace_back(std::make_pair(root->get_symbol(), code));
      return;
    }

    std::vector<bool> l = code;
    std::vector<bool> r = code;
    l.push_back(0);
    r.push_back(1);
    save_codes(root->get_left_node(), l);
    save_codes(root->get_right_node(), r);
  }

  void save_codes() { save_codes(huffman_tree.top()); }

  void build() {
    for (size_t i = 0; i < TABLE_SIZE; ++i) {
      size_t freq = frequencies[i];
      if (freq > 0)
        huffman_tree.push(new tree_node(i, freq));
    }

    while (huffman_tree.size() != 1) {
      tree_node *left = huffman_tree.top();
      huffman_tree.pop();

      tree_node *right = huffman_tree.top();
      huffman_tree.pop();

      tree_node *top =
          new tree_node(left->get_weight() + right->get_weight(), left, right);

      huffman_tree.push(top);
    }
  }

  void canonicalize(void) {
    /* Sort by lengths */

    std::sort(codes.begin(), codes.end(),
              [](const std::pair<byte_t, std::vector<bool>> &lhs,
                 const std::pair<byte_t, std::vector<bool>> &rhs) {
                return lhs.second.size() < rhs.second.size();
              });

    /* Canonicalization */
    /* First */
    std::pair<byte_t, std::vector<bool>> &s_1 = codes[0];
    size_t l_1 = s_1.second.size();
    size_t c_1 = 0;
    s_1.second.clear();
    for (size_t i = 0; i < l_1; ++i)
      s_1.second.push_back(0);

    if (codes.size() > 1) {
      /* Second */
      std::pair<byte_t, std::vector<bool>> &s_2 = codes[1];
      size_t l_2 = s_2.second.size();
      size_t c_2 = (c_1 + 1) << (l_2 - l_1);
      size_t l_i_prev = l_2;
      size_t c_i_prev = c_2;

      s_2.second.clear();
      while (c_2) {
        bool set = c_2 & 1 ? 1 : 0;
        s_2.second.push_back(set);
        c_2 >>= 1;
      }

      while (s_2.second.size() < l_2) {
        s_2.second.push_back(0);
      }

      std::reverse(s_2.second.begin(), s_2.second.end());

      /* Rest */
      for (size_t i = 2, e = codes.size(); i < e; ++i) {
        std::pair<byte_t, std::vector<bool>> &s_i = codes[i];
        size_t l_i = s_i.second.size();
        size_t c_i = (c_i_prev + 1) * (1 << (l_i - l_i_prev));

        l_i_prev = l_i;
        c_i_prev = c_i;

        s_i.second.clear();
        while (c_i) {
          bool set = c_i & 1 ? 1 : 0;
          s_i.second.push_back(set);
          c_i >>= 1;
        }

        while (s_i.second.size() < l_i) {
          s_i.second.push_back(0);
        }

        std::reverse(s_i.second.begin(), s_i.second.end());
      }
    }
  }

  void encode(FILE *output) {
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
      header[li].push_back(s_c.first);
    }

    // Encode symbols
    for (size_t i = 0; i < len; ++i)
      bp.add_bits(table[bytes[i]]);

    // Stole tree bits from nonzero_len_off
    // Store last byte padding here
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

  void dump(void) {
    for (auto &s_c : codes) {
      std::string s;
      for (bool b : s_c.second) {
        s += b ? '1' : '0';
      }
#if 1
      printf("%d[%c]: %s\n", s_c.first, s_c.first, s.c_str());
#else
      printf("%d: %s\n", s_c.first, s.c_str());
#endif
    }
  }

  ~static_huffman() {
    while (!huffman_tree.empty()) {
      tree_node *node = huffman_tree.top();
      huffman_tree.pop();
      delete node;
    }
  }
};

class adaptive_huffman {
  tree_node *root;
  tree_node *nyt_node;
  byte_t *bytes;
  size_t len;
  std::vector<bool> current_symbol_code;
  tree_node *current_symbol_node;
  size_t tree_order = 256; // size t max?

public:
  adaptive_huffman(byte_t *bytes, size_t len) : bytes{bytes}, len{len} {
    nyt_node = tree_node::create_nyt_node();
    nyt_node->set_order(tree_order--);
    root = nyt_node;
  }
  void save_code(tree_node *root, byte_t s, std::vector<bool> code) {
    if (!root) {
      // not found
      code.clear();
      while (s) {
        bool set = s & 1 ? 1 : 0;
        code.push_back(set);
        s >>= 1;
      }

      while (code.size() < CHAR_BIT) {
        code.push_back(0);
      }

      assert(nyt_node->get_weight() == 0);
      code.push_back(nyt_node->get_weight());

      std::reverse(code.begin(), code.end());
      current_symbol_code = code;
      current_symbol_node = nullptr;
      return;
    }

    if (root->is_leaf_node() && root->get_symbol() == s) {
      current_symbol_code = code;
      current_symbol_node = root;
      return;
    }

    std::vector<bool> l = code;
    std::vector<bool> r = code;
    l.push_back(0);
    r.push_back(1);
    save_code(root->get_left_node(), s, l);
    save_code(root->get_right_node(), s, r);
  }

  void encode_symbol(byte_t s) { save_code(root, s, {}); }

  void encode(FILE *output) {
    bitpacker bp;
    for (size_t i = 0; i < len; ++i) {
      byte_t c = bytes[i];
      // printf("iterace %zu. sym %hhu  %d\n", i, c, nyt_node->get_weight());
      encode_symbol(c);
      bp.add_bits(current_symbol_code);
      printf("Code for %d: ",c);
      for (bool b : current_symbol_code) 
          printf("%d ", b);
      printf("\n");
      update_tree_vitter(c);
    }

    for (byte_t b : bp.get_bytes())
      fputc(b, output);

    fputc(bp.get_remainder_bits_count(), output);
  }

  void decode(std::vector<byte_t> &decoded_bytes) {
    std::vector<bool> all_bits;
    for (size_t i = 0; i < len; ++i) {
      byte_t c = bytes[i];
      std::vector<bool> bits;
      int bit_counter = 0 ;
      while (c) {
        bool set = c & 1 ? 1 : 0;
        bits.push_back(set);
        c >>= 1;
        bit_counter++;
      }

      while (bit_counter < CHAR_BIT) {
        bits.push_back(0);
        bit_counter++;
      }
      
      std::reverse(bits.begin(), bits.end());

      for (bool b : bits) 
        all_bits.push_back(b);
    }

    for (size_t i = 0, padding = bytes[len - 1] + CHAR_BIT; i < padding; ++i)
        all_bits.pop_back();

    size_t bits_iter = all_bits.size();
    std::reverse(all_bits.begin(), all_bits.end());
    //std::vector<bool> symbol_bits;
    //root = tree_node::create_nyt_node();

    bitpacker bp;
    tree_node *node = root;
    while(bits_iter) {
        --bits_iter;
        bool b = all_bits[bits_iter];
        //node = b ? node->get_right_node() : node->get_left_node();
       // printf("bbx %d \n", bits_iter);
        if (node->is_leaf_node()) {
          
          byte_t s = node->get_symbol();
          putchar(s);
          encode_symbol(s);
          update_tree_vitter(s);

          node = root;
          continue;
        } else if (node->is_NYT_node() || (node == root && b == 0))
        {
          bitpacker byte;
          int i = CHAR_BIT;
          while (i)
          {
            bits_iter--;
            byte.add_bit(all_bits[bits_iter]);
            i--;
          }

          byte_t s = byte.get_bytes()[0];
          putchar(s);
          current_symbol_node = nullptr;
          update_tree_vitter(s);
          if (bits_iter == 0) break;
          b = all_bits[bits_iter-1];
          node = b ? root->get_right_node() : root->get_left_node();
          continue;
        } 
        node = b ? node->get_right_node() : node->get_left_node();
    }
  }

  void update_tree_vitter(byte_t s) {
    tree_node *leaf_to_increment = nullptr;
    tree_node *p = current_symbol_node;
    if (p == nullptr) {
      // Create two leaf child of the 0-node
      tree_node *left_child = new tree_node();
      tree_node *right_child = new tree_node();
      right_child->set_order(tree_order--);
      left_child->set_order(tree_order--);
      nyt_node->set_left_node(left_child);
      nyt_node->set_right_node(right_child);
      nyt_node->unset_NYT(); // not a NYT now
      // Set previous NYT node as a parent
      left_child->set_parent_node(nyt_node);
      right_child->set_parent_node(nyt_node);
      // right child is the new symbol node
      current_symbol_node = right_child;
      current_symbol_node->set_symbol(s);
      current_symbol_node->set_type(leaf);
      // left child is the new 0-node
      left_child->set_NYT(); // new NYT now
      nyt_node = left_child;
      // p = parent of the symbol node
      p = current_symbol_node->get_parent_node();
      // leafToIncrement = the right child of p;
      leaf_to_increment = p->get_right_node();
    } else {
      tree_node *leader = get_block_leader(p);
      byte_t sym = p->get_symbol();
      p->set_symbol(leader->get_symbol());
      leader->set_symbol(sym);
      std::swap(p, leader);

      tree_node *nyt_parent = nyt_node->get_parent_node();
      assert(nyt_parent && "nyt parent not exists!");
      if ((nyt_parent->get_left_node() == p &&
           nyt_parent->get_right_node() == nyt_node) ||
          (nyt_parent->get_left_node() == nyt_node &&
           nyt_parent->get_right_node() == p)) {
        leaf_to_increment = p;
        p = p->get_parent_node();
      }
    }

    while (p != root)
      p = slide_and_increment(p);

    root->set_weight(root->get_weight() + 1);

    if (leaf_to_increment != nullptr)
      slide_and_increment(leaf_to_increment);
  }

  void get_nodes(tree_node *root, node_type type, size_t weight,
                 std::vector<tree_node *> &nodes) {
    if (!root)
      return;

    if (root->get_type() == type && root->get_weight() == weight) {
      nodes.push_back(root);
    }

    get_nodes(root->get_left_node(), type, weight, nodes);
    get_nodes(root->get_right_node(), type, weight, nodes);
  }

  std::vector<tree_node *> get_block_nodes(tree_node *root, node_type type,
                                           size_t weight) {
    std::vector<tree_node *> block_nodes;
    get_nodes(root, type, weight, block_nodes);
    std::sort(block_nodes.begin(), block_nodes.end(),
              [](tree_node *lhs, tree_node *rhs) {
                return lhs->get_weight() > rhs->get_weight();
              });
    return block_nodes;
  }

  tree_node *get_block_leader(tree_node *node) {
    std::vector<tree_node *> block_nodes =
        get_block_nodes(root, node->get_type(), node->get_weight());
    return block_nodes.front();
  }
  tree_node *slide_node(tree_node *node, node_type type, size_t weight) {
    std::vector<tree_node *> block_nodes = get_block_nodes(root, type, weight);
    if (block_nodes.empty())
      return node;
    tree_node *top_right_leaf_node = block_nodes.front();
    block_nodes.insert(block_nodes.begin(), node);
    for (size_t i = block_nodes.size() - 1, e = 1; i != e; --i) {
      byte_t sym = block_nodes[i]->get_symbol();
      block_nodes[i]->set_symbol(block_nodes[i - 1]->get_symbol());
      block_nodes[i - 1]->set_symbol(sym);
    }

    return top_right_leaf_node;
  }

  tree_node *slide_and_increment(tree_node *node) {
    // fp = parent of p
    tree_node *node_parent = node->get_parent_node();
    // wt = p’s weight
    size_t node_weight = node->get_weight();

    tree_node *node_to_slide;
    if (node->is_internal_node()) {
      // Slide p in the tree higher than the leaf nodes of weight wt + 1
      node_to_slide = slide_node(node, leaf, node_weight + 1);
      assert(node_to_slide);

      if (node != node_to_slide) {
        byte_t sym = node->get_symbol();
        node_type type = node->get_type();
        size_t weight = node->get_weight();
        node->set_symbol(node_to_slide->get_symbol());
        node->set_type(node_to_slide->get_type());
        node->set_weight(node_to_slide->get_weight());
        node_to_slide->set_symbol(sym);
        node_to_slide->set_type(type);
        node_to_slide->set_weight(weight);
        tree_node::replace_node(node, node_to_slide);
      }

      node_to_slide->set_weight(node_weight + 1);
      node = node_parent;
    } else {
      // Slide p in the tree higher than the internal nodes of weight wt
      node_to_slide = slide_node(node, internal, node_weight);
      assert(node_to_slide);

      if (node != node_to_slide) {
        byte_t sym = node->get_symbol();
        node_type type = node->get_type();
        size_t weight = node->get_weight();
        node->set_symbol(node_to_slide->get_symbol());
        node->set_type(node_to_slide->get_type());
        node->set_weight(node_to_slide->get_weight());
        node_to_slide->set_symbol(sym);
        node_to_slide->set_type(type);
        node_to_slide->set_weight(weight);
        tree_node::replace_node(node, node_to_slide);
      }

      node_to_slide->set_weight(node_weight + 1);
      node = node_to_slide->get_parent_node();
    }

    return node;
  }
};

int main(int argc, char **argv) {
  int compress_mode = invalid;
  const char *model = NULL;
  char *in = NULL;
  char *out = NULL;
  const char *huffman_mode = NULL;

  int c;
  while ((c = getopt(argc, argv, "cdi:o:mh:")) != -1) {
    switch (c) {
    case 'c':
      compress_mode = encoding;
      break;
    case 'd':
      compress_mode = decoding;
      break;
    case 'i':
      in = optarg;
      break;
    case 'o':
      out = optarg;
      break;
    case 'm':
      model = "pix_diff";
      break;
    case 'h':
      if (strcmp(optarg, "static") == 0 || strcmp(optarg, "adaptive") == 0) {
        huffman_mode = optarg;
      } else {
        fprintf(stderr, "Unknown Huffman mode '%s'.\n", optarg);
        return 1;
      }
      break;
    case '?':
      if (optopt == 'h') {
        puts("help");
        return 0;
      } else {
        fprintf(stderr, "Unknown option '%c'.\n", optopt);
        return 1;
      }
    default:
      fprintf(stderr, "Invalid arguments\n");
      return 1;
    }
  }

  FILE *input = fopen(in, "r");
  if (!input) {
    fprintf(stderr, "Input file does not exists.\n");
    return 1;
  }
  fseek(input, 0L, SEEK_END);
  size_t count = ftell(input);
  fseek(input, 0L, SEEK_SET);

  byte_t *bytes = (byte_t *)malloc(sizeof(byte_t) * count);
  size_t len = fread(bytes, sizeof(byte_t), count, input);
  fclose(input);

  FILE *output = fopen(out, "w");

  if (compress_mode == encoding) {
    if (model && strcmp(model, "pix_diff") == 0) {
      apply_pixel_diff_model(bytes, len);
    }

    if (strcmp(huffman_mode, "static") == 0) {
      static_huffman sh{bytes, len};
      sh.build();
      sh.save_codes();
      sh.canonicalize();
      sh.encode(output);
    }
    if (strcmp(huffman_mode, "adaptive") == 0) {
      adaptive_huffman ah{bytes, len};
      ah.encode(output);
    }
  } else if (compress_mode == decoding) {
    std::vector<byte_t> decoded_bytes;
    if (strcmp(huffman_mode, "static") == 0) {
      decoder dc{bytes, len};
      dc.fcfs();
      dc.fastCHD(decoded_bytes);
    }
    if (strcmp(huffman_mode, "adaptive") == 0) {
      adaptive_huffman ah{bytes, len};
      ah.decode(decoded_bytes);
    }

    if (model && strcmp(model, "pix_diff") == 0) {
      byte_t *bytes = decoded_bytes.data();
      size_t len = decoded_bytes.size();
      reverse_pixel_diff_model(bytes, len);
    }

    if (strcmp(huffman_mode, "static") == 0) {
      for (byte_t b : decoded_bytes)
        fputc(b, output);
    }
  }

  fclose(output);
  free(bytes);
  return 0;
}