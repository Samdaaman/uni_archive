#!/bin/bash
# Adapted from https://unix.stackexchange.com/a/52347

bin=main
iterations=300

gcc -o $bin.o -O3 -Ofast $bin.c

echo "Using $iterations iterations"

for n in 21 31 51 101 151 201 301 401 501 601 701 801 901
do
    sleep 3
    start=`date +%s.%N`
    ./$bin.o $n $iterations > /dev/null 2> /dev/null
    end=`date +%s.%N`
    echo -n "[${n}x${n}x${n}] Time: "
    echo "$end - $start" | bc -l
done
