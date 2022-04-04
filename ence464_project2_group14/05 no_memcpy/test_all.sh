function test_with_flags() {
    echo "Testing $1"
    ./test.sh "$1"
}

test_with_flags '-DONE_CALLOC'
test_with_flags '-DONE_CALLOC_MALLOC'
test_with_flags '-DONE_MALLOC'
test_with_flags '-DNO_HEAP -mcmodel=large'
test_with_flags '-DFOUR_THREADS'
test_with_flags