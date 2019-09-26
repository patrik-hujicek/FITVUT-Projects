#!/bin/bash
STR=$1
mpic++ --prefix /usr/local/share/OpenMPI -std=c++17 -Wall -Wextra -O3 vuv.cpp -o vuv
len=${#STR} 
p=$((len == 1 ? 1 : 2*len-2))
mpirun --prefix /usr/local/share/OpenMPI -np $p vuv $STR
rm -f vuv
