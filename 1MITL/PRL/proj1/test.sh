#!/bin/bash

if [ "$#" -eq 1 ];
then
    n=$1
else
    echo "Missing number of elements! Usage: ./tesh.sh numbers_count"
    exit 1
fi

if [ $n -le 0 ];
then
    echo "Invalid number of elements to sort!"
    exit 1
fi

dd if=/dev/random bs=1 count=$n of=numbers status=none

mpic++ --prefix /usr/local/share/OpenMPI -std=c++17 -Wall -Wextra -O3 bks.cpp -o bks

if [ $n -eq 1 ];
then
    p=1
else
    p=`python3 -c "import math; print(int(2 * math.pow(2, math.ceil(math.log2(math.log2($n))))-1))"`
fi

mpirun --prefix /usr/local/share/OpenMPI -np $p bks
rm -f bks numbers
