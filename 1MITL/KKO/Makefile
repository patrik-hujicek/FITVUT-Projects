# Author: David Bolvansky
# Login: xbolva00
# Date: 8. 3. 2019
# Filename: Makefile
# Description: Program building rules

CC=g++
CFLAGS= -std=c++11 -Wall -Wextra -O3 -flto -static
OUT=huff_codec

all: $(OUT)
debug:
	$(CC) -std=c++11 -Wall -Wextra -ggdb3 huffman.cpp static_huffman.cpp adaptive_huffman.cpp pixel_diff_model.cpp tree_node.cpp bitpacker.cpp -o $(OUT)

bitpacker.o: bitpacker.cpp bitpacker.h constants.h
	$(CC) $(CFLAGS) -c $< -o $@
tree_node.o: tree_node.cpp tree_node.h constants.h
	$(CC) $(CFLAGS) -c $< -o $@
pixel_diff_model.o: pixel_diff_model.cpp pixel_diff_model.h constants.h
	$(CC) $(CFLAGS) -c $< -o $@
static_huffman.o: static_huffman.cpp static_huffman.h bitpacker.h constants.h tree_node.h
	$(CC) $(CFLAGS) -c $< -o $@
adaptive_huffman.o: adaptive_huffman.cpp adaptive_huffman.h bitpacker.h constants.h tree_node.h
	$(CC) $(CFLAGS) -c $< -o $@
huffman.o: huffman.cpp bitpacker.h constants.h tree_node.h pixel_diff_model.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): huffman.o static_huffman.o adaptive_huffman.o pixel_diff_model.o tree_node.o bitpacker.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(OUT)

pack:
	zip -r kko_xbolva00.zip *.cpp *.h Makefile dokumentacia.pdf

format:
	clang-format -i *.cpp *.h -style=LLVM
