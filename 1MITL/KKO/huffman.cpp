/*
 * Author: David Bolvansky
 * Login: xbolva00
 * Date: 8. 3. 2019
 * Filename: huffman.cpp
 * Description: Main program
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adaptive_huffman.h"
#include "pixel_diff_model.h"
#include "static_huffman.h"

const char *USAGE =
    "Usage: ./huff_codec options\n\nOptions:\n-c\tcompress file\n-d\tdecompress"
    " file\n-i file\tinput file\n-o file\toutput file\n-h name\thuffman mode "
    "('static' or 'adaptive')\n-m name\tprocessing model (default: 'pix_diff')"
    "\n-h\tprint help";

/*
 * Main function
 * \param argc number of arguments
 * \param argv arguments
 * \return status code
 */
int main(int argc, char **argv) {
  int compress_mode = invalid;
  const char *model = NULL;
  char *in_filename = NULL;
  char *out_filename = NULL;
  const char *huffman_mode = NULL;

  if (argc == 2) {
    if (strcmp(argv[1], "-h") == 0) {
      puts(USAGE);
      return 0;
    }
  }

  int c;
  while ((c = getopt(argc, argv, "cdi:o:mh:w:")) != -1) {
    switch (c) {
    case 'c':
      compress_mode = encoding;
      break;
    case 'd':
      compress_mode = decoding;
      break;
    case 'i':
      in_filename = optarg;
      break;
    case 'o':
      out_filename = optarg;
      break;
    case 'w':
      // unused
      break;
    case 'm':
      model = "pix_diff";
      break;
    case 'h':
      if (strcmp(optarg, "static") == 0 || strcmp(optarg, "adaptive") == 0) {
        huffman_mode = optarg;
      } else {
        fprintf(stderr, "Unknown Huffman mode '%s'.\n%s'\n", optarg, USAGE);
        return 1;
      }
      break;
    case '?':
      fprintf(stderr, "%s\n", USAGE);
      return 1;
    default:
      fprintf(stderr, "%s\n", USAGE);
      return 1;
    }
  }

  if (!in_filename) {
    fprintf(stderr, "Input file is mandatory.\n");
    return 1;
  }

  if (!out_filename) {
    fprintf(stderr, "Output file is mandatory.\n");
    return 1;
  }

  if (compress_mode == invalid) {
    fprintf(stderr, "Compress mode is mandatory.\n");
    return 1;
  }

  FILE *input = fopen(in_filename, "r");
  if (!input) {
    fprintf(stderr, "Input file does not exists.\n");
    return 1;
  }

  // Get file size
  fseek(input, 0L, SEEK_END);
  size_t count = ftell(input);
  fseek(input, 0L, SEEK_SET);

  // Read file
  byte_t *bytes = (byte_t *)malloc(sizeof(byte_t) * count);
  size_t len = fread(bytes, sizeof(byte_t), count, input);
  fclose(input);

  FILE *output = fopen(out_filename, "w");
  if (!output) {
    fprintf(stderr, "Cannot open output file.\n");
    return 1;
  }

  // Compress / decompress file
  if (compress_mode == encoding) {
    if (model && strcmp(model, "pix_diff") == 0) {
      apply_pixel_diff_model(bytes, len);
    }

    if (strcmp(huffman_mode, "static") == 0) {
      static_huffman sh{bytes, len};
      sh.encode(output);
    }
    if (strcmp(huffman_mode, "adaptive") == 0) {
      adaptive_huffman ah{bytes, len};
      ah.encode(output);
    }
  } else if (compress_mode == decoding) {
    std::vector<byte_t> decoded_bytes;
    if (strcmp(huffman_mode, "static") == 0) {
      static_huffman sh{bytes, len};
      sh.decode(decoded_bytes);
    }
    if (strcmp(huffman_mode, "adaptive") == 0) {
      adaptive_huffman ah{bytes, len};
      ah.decode(decoded_bytes);
    }

    if (model && strcmp(model, "pix_diff") == 0) {
      reverse_pixel_diff_model(decoded_bytes.data(), decoded_bytes.size());
    }

    for (byte_t b : decoded_bytes)
      fputc(b, output);
  }

  // Release resources
  fclose(output);
  free(bytes);
  return 0;
}