CC=gcc
CFLAGS=-std=c99 -Wall -Werror -Wextra -pedantic -O2
LDFLAGS=-lelf

all: cda-proj1

cda-proj1.o: cda-proj1.c cda-proj1.h section.h
	$(CC) $(CFLAGS) -c $< -o $@
section.o: section.c section.h
	$(CC) $(CFLAGS) -c $< -o $@

cda-proj1: cda-proj1.o section.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o cda-proj1

pack: clean
	tar cvf xbolva00.tar ../src/