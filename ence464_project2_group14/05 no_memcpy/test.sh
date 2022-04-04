#!/bin/bash

bin=main
iterations=300

if gcc -o $bin.o -DPRINT_OUTPUT $1 $bin.c -lpthread; then
    echo "[$1] Compiled successfully"
else
    exit 2
fi

status=0

for test in 15 21 31 41 51
do
    if ./$bin.o $test $iterations | cmp ../reference2/$test.txt; then
        # echo "===> [TEST] n=$test i=$iterations correct"
        :
    else
        echo "===> [TEST] n=$test i=$iterations failed!"
        status=1
    fi
done

if [ "$status" == "0" ]; then
    echo "[$1] Test success"
else
    echo "[$1] Test Failed"
fi

echo ""
exit $status
