#!/bin/bash


if [ "$1" == "test" ]; then
    debug=1
    end=1
else
    end=10
fi


function benchmark_with_flags() {
    if [ "$debug" == "1" ]; then
        cd "$1"
        ./test.sh "$2"
        cd ..
    else
        python3 benchmark.py "$1" "gcc -o main.o -DPRINT_TIME $2 main.c -lpthread" "ESL24_$1_$2"
    fi
}

# Runs some benchmarks for data
for i in $(seq 1 $end)
do
    echo "Test #$i"

    # # ------------------------
    # # Early versions
    # # ------------------------
    # # 00 python3 version
    # python3 benchmark.py '00 python3 version' 'echo GCC is for noobs python3 is the way' '00_ESL29'

    # # 01 first_attempt (basic C version with little optimisations)
    # python3 benchmark.py '01 first_attempt' 'gcc -o main.o -O3 -Ofast -DPRINT_TIME main.c' '01_ESL29'

    # # 02 implemented slice algorithm
    # python3 benchmark.py '02 slicey_slice' 'gcc -o main.o -O3 -Ofast -DPRINT_TIME main.c' '02_ESL29'

    # # 03 version (as before but uses floats)
    # python3 benchmark.py '03 slicey_slice_float' 'gcc -o main.o -O3 -Ofast -DPRINT_TIME main.c' '03_ESL29'

    # # Quad thread (04 version)
    # python3 benchmark.py '04 slicey_slice_threaded' 'gcc -o main.o -O3 -Ofast -DPRINT_TIME -DNUM_THREADS=4 main.c -lpthread' '04_ESL29_QUAD'
    
    # # Octa thread (04 version)
    # python3 benchmark.py '04 slicey_slice_threaded' 'gcc -o main.o -O3 -Ofast -DPRINT_TIME -DNUM_THREADS=8 main.c -lpthread' '04_ESL29_OCTA'

    # # ------------------------
    # # Current version
    # # ------------------------
    # # Quad thread no_memcpy (05 version)
    # python3 benchmark.py '05 no_memcpy' 'gcc -o main.o -O3 -Ofast -DPRINT_TIME -DNUM_THREADS=4 main.c -lpthread' '05_ESL29_QUAD'
    
    # # Octa thread no_memcpy (05 version)
    # python3 benchmark.py '05 no_memcpy' 'gcc -o main.o -O3 -Ofast -DPRINT_TIME -DNUM_THREADS=8 main.c -lpthread' '05_ESL29_OCTA'

    # # Fine tuning --------------------
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DONE_CALLOC'
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DONE_CALLOC_MALLOC'
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DONE_MALLOC'
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DNO_HEAP -mcmodel=large'
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DFOUR_THREADS'
    # benchmark_with_flags '05 no_memcpy'

    # # Benchmarking optimisations
    # benchmark_with_flags '05 no_memcpy' ''
    # benchmark_with_flags '05 no_memcpy' '-O1'
    # benchmark_with_flags '05 no_memcpy' '-O2'
    # benchmark_with_flags '05 no_memcpy' '-O3'
    # benchmark_with_flags '05 no_memcpy' '-Ofast'
    # benchmark_with_flags '05 no_memcpy' '-Og'
    # benchmark_with_flags '05 no_memcpy' '-Os'
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast' # optimal version

    # # Benchmark data type
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DUSE_DOUBLE'

    # # Benchmark effectivness of removing memcpy
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DUSE_MEMCPY'

    # # Benchmark number of threads
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DFOUR_THREADS'
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DONE_THREAD'

    # # Benchmark effectivness `of inlining functions
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DNO_INLINE'

    # # New 06 version
    # benchmark_with_flags '06 hackity_hack' '-O3 -Ofast'

    # Benchmarking cache hit
    # benchmark_with_flags '01 first_attempt' '-O3 -Ofast'
    # benchmark_with_flags '05 no_memcpy' '-O3 -Ofast'


    benchmark_with_flags '05 no_memcpy' '-O1'
    benchmark_with_flags '05 no_memcpy' '-O2'
    benchmark_with_flags '05 no_memcpy' '-O3'
    benchmark_with_flags '05 no_memcpy' '-O3 -Ofast'
    benchmark_with_flags '05 no_memcpy' '-O1 -DUSE_DOUBLE'
    benchmark_with_flags '05 no_memcpy' '-O2 -DUSE_DOUBLE'
    benchmark_with_flags '05 no_memcpy' '-O3 -DUSE_DOUBLE'
    benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DUSE_DOUBLE'
    benchmark_with_flags '05 no_memcpy' '-O3 -DFOUR_THREADS'
    benchmark_with_flags '05 no_memcpy' '-O3 -Ofast -DFOUR_THREADS'

    benchmark_with_flags '06 hackity_hack' '-O1'
    benchmark_with_flags '06 hackity_hack' '-O2'
    benchmark_with_flags '06 hackity_hack' '-O3'
    benchmark_with_flags '06 hackity_hack' '-O3 -Ofast'
    benchmark_with_flags '06 hackity_hack' '-O1 -DUSE_DOUBLE'
    benchmark_with_flags '06 hackity_hack' '-O2 -DUSE_DOUBLE'
    benchmark_with_flags '06 hackity_hack' '-O3 -DUSE_DOUBLE'
    benchmark_with_flags '06 hackity_hack' '-O3 -Ofast -DUSE_DOUBLE'
    benchmark_with_flags '06 hackity_hack' '-O3 -DFOUR_THREADS'
    benchmark_with_flags '06 hackity_hack' '-O3 -Ofast -DFOUR_THREADS'
done
