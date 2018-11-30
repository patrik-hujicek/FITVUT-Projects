# Architektura procesoru (ACH 2018)
# Projekt c. 1 (nbody)
# Login: xbolva00

CC=icpc
CFLAGS=-std=c++11 -lpapi -ansi-alias
OPT=-O2 -Wall
#REPORT=-qopt-report=5

N=1500
DT=0.001f
STEPS=750

PARAMS=-DN=$(N) -DDT=$(DT) -DSTEPS=$(STEPS)

INPUT=../input.dat
OUTPUT=../step0.dat

PAPI_EVENTS=PAPI_FP_OPS|PAPI_SP_OPS

all:
	$(CC) $(CFLAGS) $(OPT) $(REPORT) $(PARAMS) -c velocity.cpp
	$(CC) $(CFLAGS) $(OPT) $(REPORT) $(PARAMS) -c nbody.cpp
	$(CC) $(CFLAGS) $(OPT) $(PARAMS) velocity.o nbody.o main.cpp -o nbody
	$(CC) $(CFLAGS) gen.cpp -o gen

clean:
	rm -f *.o *.optrpt *.s gen nbody

run:
	PAPI_EVENTS='$(PAPI_EVENTS)' ./nbody $(INPUT) $(OUTPUT)
