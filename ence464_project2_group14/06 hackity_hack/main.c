#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


// Optional compiler defines ---------------
// #define NO_INLINE
// #define FOUR_THREADS
// #define USE_DOUBLES
// -----------------------------------------

#define DELTA 1.0

#ifdef EIGHT_THREADS
    #define NUM_THREADS 8
#else
    #define NUM_THREADS 4
#endif

#ifdef USE_DOUBLE
    typedef double data_t;
#else
    typedef float data_t;
#endif

#ifdef NO_INLINE
    #define __conditional_inline
#else
    #define __conditional_inline __always_inline
#endif
typedef struct {
    data_t* __restrict__ z_minus_1_face;
    data_t* __restrict__ z_face;
    data_t* __restrict__ z_plus_1_face;
    data_t* __restrict__ source_face;
    data_t* __restrict__ next_z_face;
} slice_t;

typedef struct {
    slice_t* slice_start;
    unsigned int slice_count;
    unsigned int thread_index;
} thread_work_block_t;

#ifdef PRINT_TIME
static struct timespec start_time;
static struct timespec end_time;
#endif

static unsigned int n;
static unsigned int n_minus_1;
static unsigned int n_squared;
static size_t n_cubed_bytes;
static unsigned int iterations;
static unsigned int offset_negative_boundary;
static unsigned int offset_postive_boundary;
static thread_work_block_t thread_work_blocks[NUM_THREADS];

#ifndef ONE_THREAD
    static sem_t thread_done_iteration_semaphores[NUM_THREADS - 1];
    static sem_t pointer_swap_done_semaphores[NUM_THREADS - 1];
#endif

static data_t* __restrict__ source;
static data_t* __restrict__ current;
static data_t* __restrict__ next;
static slice_t* __restrict__ slices;


__conditional_inline static void process_slice_partial(slice_t slice) {
    for (unsigned int y = offset_negative_boundary; y < offset_postive_boundary; y++) {
        for (unsigned int x = offset_negative_boundary; x < offset_postive_boundary; x++) {
            slice.next_z_face[y*n + x] = 1 / 6.0 * ( \
                slice.z_minus_1_face[y*n + x] + \
                slice.z_face[y*n + x - n] + \
                slice.z_face[y*n + x - 1] + \
                slice.z_face[y*n + x + 1] + \
                slice.z_face[y*n + x + n] + \
                slice.z_plus_1_face[y*n + x] + \
                - slice.source_face[y*n + x] \
            );
        }
    }

}


__conditional_inline static void process_slice(slice_t slice) {
    // x=0 & y=0 corner
    slice.next_z_face[0] = 1 / 6.0 * ( \
        slice.z_minus_1_face[0] + \
        2 * slice.z_face[1] + \
        2 * slice.z_face[n] + \
        slice.z_plus_1_face[0] + \
        - slice.source_face[0] \
    );

    // y=0 & 0<x<n_minus_1 edge
    for (unsigned int x = 1; x < n_minus_1; x++) {
        slice.next_z_face[x] = 1 / 6.0 * ( \
            slice.z_minus_1_face[x] + \
            slice.z_face[x - 1] + \
            slice.z_face[x + 1] + \
            2 * slice.z_face[x + n] + \
            slice.z_plus_1_face[x] + \
            - slice.source_face[x] \
        );
    }

    // y=0 & x=n_minus_1 corner
    slice.next_z_face[n_minus_1] = 1 / 6.0 * ( \
        slice.z_minus_1_face[n_minus_1] + \
        2 * slice.z_face[n_minus_1 - 1] + \
        2 * slice.z_face[n_minus_1 + n] + \
        slice.z_plus_1_face[n_minus_1] + \
        - slice.source_face[n_minus_1] \
    );

    for (unsigned int y = 1; y < n_minus_1; y++) {
        // 0<y<n_minus_1 & x=0 edge
        slice.next_z_face[y*n] = 1 / 6.0 * ( \
            slice.z_minus_1_face[y*n] + \
            slice.z_face[y*n - n] + \
            2*slice.z_face[y*n + 1] + \
            slice.z_face[y*n + n] + \
            slice.z_plus_1_face[y*n] + \
            - slice.source_face[y*n] \
        );

        // 0<y<n_minus_1 & 0<x<n_minus_1 (middle section with no BCs)
        for (unsigned int x = 1; x < n_minus_1; x++) {
            slice.next_z_face[y*n + x] = 1 / 6.0 * ( \
                slice.z_minus_1_face[y*n + x] + \
                slice.z_face[y*n + x - n] + \
                slice.z_face[y*n + x - 1] + \
                slice.z_face[y*n + x + 1] + \
                slice.z_face[y*n + x + n] + \
                slice.z_plus_1_face[y*n + x] + \
                - slice.source_face[y*n + x] \
            );
        }

        // 0<y<n_minus_1 & x=n_minus_1 edge
        slice.next_z_face[y*n + n_minus_1] = 1 / 6.0 * ( \
            slice.z_minus_1_face[y*n + n_minus_1] + \
            slice.z_face[y*n - n + n_minus_1] + \
            2*slice.z_face[y*n + n_minus_1 - 1] + \
            slice.z_face[y*n + n + n_minus_1] + \
            slice.z_plus_1_face[y*n + n_minus_1] + \
            - slice.source_face[y*n + n_minus_1] \
        );
    }

    // y=n_minus_1 & x=0 corner
    slice.next_z_face[n_minus_1 * n] = 1 / 6.0 * ( \
        slice.z_minus_1_face[n_minus_1 * n] + \
        2 * slice.z_face[n_minus_1 * n - n] + \
        2 * slice.z_face[n_minus_1 * n + 1] + \
        slice.z_plus_1_face[n_minus_1 * n] + \
        - slice.source_face[n_minus_1 * n] \
    );

    // y=n_minus_1 & 0<x<n_minus_1 edge
    for (unsigned int x = 1; x < n_minus_1; x++) {
        slice.next_z_face[n_minus_1 * n + x] = 1 / 6.0 * ( \
            slice.z_minus_1_face[n_minus_1 * n + x] + \
            2 * slice.z_face[n_minus_1 * n - n + x] + \
            slice.z_face[n_minus_1 * n + x - 1] + \
            slice.z_face[n_minus_1 * n + x + 1] + \
            slice.z_plus_1_face[n_minus_1 * n + x] + \
            - slice.source_face[x] \
        );
    }

    // y=n_minus_1 & x=n_minus_1 corner
    slice.next_z_face[n_minus_1 * n + n_minus_1] = 1 / 6.0 * ( \
        slice.z_minus_1_face[n_minus_1 * n + n_minus_1] + \
        2 * slice.z_face[n_minus_1 * n - n + n_minus_1] + \
        2 * slice.z_face[n_minus_1 * n + n_minus_1 - 1] + \
        slice.z_plus_1_face[n_minus_1 * n + n_minus_1] + \
        - slice.source_face[n_minus_1 * n + n_minus_1] \
    );
}


#ifdef USE_MEMCPY
// Extracted to a function so we can profile this bad boi
__conditional_inline static void our_memcpy(void* __restrict__ dst, void* __restrict__ src, size_t n) {
    memcpy(dst, src, n);
}
#endif

void* process_thread_work_block(void* args) {
    thread_work_block_t* thread_work_block = args;

    for (unsigned int iteration = 0; iteration < iterations; iteration++) {
        sem_wait(pointer_swap_done_semaphores + thread_work_block->thread_index);
        
        if (offset_negative_boundary == 0) {
            for (slice_t* slice = (thread_work_block)->slice_start; slice < thread_work_block->slice_start + thread_work_block->slice_count; slice++) { // scary for-loop that increases a pointer
                process_slice(*slice);
            }
        } else {
            for (slice_t* slice = (thread_work_block)->slice_start; slice < thread_work_block->slice_start + thread_work_block->slice_count; slice++) { // scary for-loop that increases a pointer
                process_slice_partial(*slice);
            }
        }

        sem_post(thread_done_iteration_semaphores + thread_work_block->thread_index);
    }
}

__conditional_inline static unsigned int run_partial() {
    pthread_t threads[NUM_THREADS - 1];
    for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
        thread_work_blocks[i].thread_index = i;
        pthread_create(threads + i, NULL, process_thread_work_block, thread_work_blocks + i);
    }

    for (unsigned int iteration = 0; iteration < iterations; iteration++) {
        unsigned int number_of_layers = offset_postive_boundary - offset_negative_boundary;
        for (unsigned int layer = 0; layer < number_of_layers; layer++) {
            const unsigned int z_offset = (offset_negative_boundary + layer) * n_squared;
            slices[layer] = (slice_t){
                .z_minus_1_face = current + z_offset - n_squared,
                .z_face = current + z_offset,
                .z_plus_1_face = current + z_offset + n_squared,
                .next_z_face = next + z_offset,
                .source_face = source + z_offset,
            };
        }

        // Evenly divide up slices to be processed amoung the thread_work_blocks array
        unsigned int current_slice = 0;
        for (unsigned int i = 0; i < NUM_THREADS; i++) {
            unsigned int count = number_of_layers / NUM_THREADS;
            if (number_of_layers % NUM_THREADS > i) {
                count++;
            }

            thread_work_blocks[i].slice_start = slices + current_slice;
            thread_work_blocks[i].slice_count = count;

            current_slice += count;
        }

        // If we are not on inner cubes return the current iteration
        if (number_of_layers == n) {
            return iteration;
        }

        // Unblock worker threads
        for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
            sem_post(pointer_swap_done_semaphores + i);
        }

        // Process slices on the main thread
        for (slice_t* slice = thread_work_blocks[NUM_THREADS - 1].slice_start; slice < thread_work_blocks[NUM_THREADS - 1].slice_start + thread_work_blocks[NUM_THREADS - 1].slice_count; slice++) {
            process_slice_partial(*slice);
        }

        // Wait for other threads to finish
        for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
            sem_wait(thread_done_iteration_semaphores + i);
        }

        // Swap current and next pointers
        data_t *temp = current;
        current = next;
        next = temp;
        
        offset_negative_boundary--;
        offset_postive_boundary++;
    }
    return iterations; // We are done
}

__conditional_inline static void run() {
    const unsigned int current_iteration = run_partial();

    // Boundary conditions on z layer
    slices[0].z_minus_1_face = slices[0].z_plus_1_face;
    slices[n_minus_1].z_plus_1_face = slices[n_minus_1].z_minus_1_face;

    for (unsigned int i = current_iteration; i < iterations; i++) {
        for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
            sem_post(pointer_swap_done_semaphores + i);
        }

        // Process slices on the main thread
        for (unsigned int i = n - n / NUM_THREADS; i < n; i++) {
            process_slice(slices[i]);
        }

        // Wait for other threads to finish
        for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
            sem_wait(thread_done_iteration_semaphores + i);
        }

        // Swap current and next pointers
        data_t *temp = current;
        current = next;
        next = temp;

        // First slice (boundary case)
        slices[0].z_minus_1_face = current + n_squared;
        slices[0].z_plus_1_face = current + n_squared;
        slices[0].z_face = current;
        slices[0].next_z_face = next;

        // Middle slices (0<z<n-1)
        for (unsigned int z = 1; z < n_minus_1; z++) {
            const unsigned int z_offset = z*n_squared;
            slices[z].z_minus_1_face = current + z_offset - n_squared;
            slices[z].z_face = current + z_offset;
            slices[z].z_plus_1_face = current + z_offset + n_squared;
            slices[z].next_z_face = next + z_offset;
        }

        // Last slice (boundary case)
        const unsigned int z_offset = (n-1)*n_squared;
        slices[n_minus_1].z_minus_1_face = current + z_offset - n_squared;
        slices[n_minus_1].z_face = current + z_offset;
        slices[n_minus_1].z_plus_1_face = current + z_offset - n_squared;
        slices[n_minus_1].next_z_face = next + z_offset;
    }
}


int main(int argc, char **argv) {
    #ifdef PRINT_TIME
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    #endif

    if (argc != 3) {
        fprintf(stderr, "[Error]: wrong arguments must be <n> <iterations>\n");
        return EXIT_FAILURE;
    }

    n = atoi(argv[1]);
    n_minus_1 = n - 1;
    n_squared = n * n;
    n_cubed_bytes = n_squared * n * sizeof(data_t);
    iterations = atoi(argv[2]);

    #ifndef ONE_THREAD
        for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
            sem_init(thread_done_iteration_semaphores + i, 0, 0);
            sem_init(pointer_swap_done_semaphores + i, 0, 0);
        }
    #endif
    
    // One calloc and one malloc (a potentially very minimal gain in speed)
    source = (data_t*)calloc(n_cubed_bytes * 3 + n * sizeof(slice_t), 1);
    current = (data_t*)(((uint8_t*)source) + n_cubed_bytes);
    next = (data_t*)(((uint8_t*)source) + 2 * n_cubed_bytes);
    slices = (slice_t*)(((uint8_t*)source) + 3 * n_cubed_bytes);
    
    // Multiple source by DELTA ^ 2 outside of loops
    source[(n*n*n) / 2] = DELTA * DELTA * 1;

    // These must both be the same distance from n / 2 !!!
    offset_negative_boundary = (n-1) / 2; // for n=901 this is 450
    offset_postive_boundary = (n+1) / 2; // for n=901 this is 451

    run();

    #ifndef USE_MEMCPY
        // if (iterations % 2 == 0) {
            // Swap current and next pointers back
            data_t *temp = current;
            current = next;
            next = temp;
        // }
    #endif


    #ifdef PRINT_OUTPUT_OLD
        // Old "reference" way
        // Print out the middle slice (ie Z cross-section) of the cube for validation
        for (int x = 0; x < n; ++x) {
            for (int y = 0; y < n; ++y) {
                printf("%0.5f ", next[((n/2) * n + y) * n + x]);
            }
            printf("\n");
        }

    #else
        #ifdef PRINT_OUTPUT
        // New "reference2" way
        // Print out the 1st, 2nd, 3rd, 4th, middle-1, middle, middle+1, 2nd last, last slices
            for (int z = 0; z < n; z++) {
                if (z < 4 || z == n/2-1 || z == n/2 || z == n/2+1 || z > n-3) {
                    printf("--------------------z=%i--------------------\n", z);
                    for (int x = 0; x < n; ++x) {
                        for (int y = 0; y < n; ++y) {
                            printf("%0.5f ", next[(z * n + y) * n + x]);
                        }
                        printf("\n");
                    }
                }
            }
        #endif
    #endif

    // Free pointers
    free(source);

    #ifdef PRINT_TIME
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    printf("%ld ", end_time.tv_nsec - start_time.tv_nsec);
    printf("%ld", end_time.tv_sec - start_time.tv_sec);
    #endif

    return EXIT_SUCCESS;
}