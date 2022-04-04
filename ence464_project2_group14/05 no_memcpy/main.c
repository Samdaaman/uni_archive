#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// Optional compiler defines ---------------
// #define NO_INLINE
// #define ONE_THREAD
// #define FOUR_THREADS
// #define USE_DOUBLES
// #define USE_MEMCPY
// -----------------------------------------

#define DELTA 1.0

#ifndef ONE_THREAD
    #ifdef FOUR_THREADS
        #define NUM_THREADS 4
    #else
        #define NUM_THREADS 8
    #endif
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
    sem_t* thread_done_iteration_semaphore;
    sem_t* pointer_swap_done_semaphore;
} thread_work_block_t;


#ifdef PRINT_TIME
static struct timespec start_time;
static struct timespec end_time;
#endif

// TODO test if these are faster as parameters rather than static variables
static unsigned int n;
static unsigned int n_minus_1;
static unsigned int n_squared;
static size_t n_cubed_bytes;
static unsigned int iterations;

#ifndef ONE_THREAD
    static sem_t thread_done_iteration_semaphores[NUM_THREADS - 1];
    static sem_t pointer_swap_done_semaphores[NUM_THREADS - 1];
#endif

static data_t* __restrict__ source;
static data_t* __restrict__ current;
static data_t* __restrict__ next;
static slice_t* __restrict__ slices;

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

#ifdef ONE_THREAD

__conditional_inline static void run() {
    // First slice (boundary case)
    slices[0] = (slice_t){
        .z_face = current,
        .z_minus_1_face = current + n_squared,
        .z_plus_1_face = current + n_squared,
        .next_z_face = next,
        .source_face = source,
    };

    // Middle slices (0<z<n-1)
    for (unsigned int z = 1; z < n_minus_1; z++) {
        const unsigned int z_offset = z*n_squared;
        slices[z] = (slice_t){
            .z_minus_1_face = current + z_offset - n_squared,
            .z_face = current + z_offset,
            .z_plus_1_face = current + z_offset + n_squared,
            .next_z_face = next + z_offset,
            .source_face = source + z_offset,
        };
    }

    // Last slice (boundary case)
    const unsigned int z_offset = (n-1)*n_squared;
    slices[n_minus_1] = (slice_t){
        .z_minus_1_face = current + z_offset - n_squared,
        .z_plus_1_face = current + z_offset - n_squared,
        .z_face = current + z_offset,
        .next_z_face = next + z_offset,
        .source_face = source + z_offset,
    };

    for (unsigned int i = 0; i < iterations - 1; i++) {
        for (slice_t* slice = slices; slice < slices + n; slice++) {
            process_slice(*slice);
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

    // Last iteration (no need to swap pointers)
    for (slice_t* slice = slices; slice < slices + n; slice++) {
        process_slice(*slice);
    }
}

#else
void* process_thread_work_block(void* thread_work_block) {
    // For all iterations except the last
    for (unsigned int iteration = 0; iteration < iterations - 1; iteration++) {
        for (slice_t* slice = ((thread_work_block_t*)thread_work_block)->slice_start; slice < ((thread_work_block_t*)thread_work_block)->slice_start + ((thread_work_block_t*)thread_work_block)->slice_count; slice++) { // scary for-loop that increases a pointer
            process_slice(*slice);
        }
        sem_post(((thread_work_block_t*)thread_work_block)->thread_done_iteration_semaphore);
        sem_wait(((thread_work_block_t*)thread_work_block)->pointer_swap_done_semaphore);
    }

    // Last iteration
    for (slice_t* slice = ((thread_work_block_t*)thread_work_block)->slice_start; slice < ((thread_work_block_t*)thread_work_block)->slice_start + ((thread_work_block_t*)thread_work_block)->slice_count; slice++) { // scary for-loop that increases a pointer
        process_slice(*slice);
    }
    // Don't worry about any semaphores as the main thread joins all the threads to make sure they terminate
}


__conditional_inline static void run() {
    // First slice (boundary case)
    slices[0] = (slice_t){
        .z_face = current,
        .z_minus_1_face = current + n_squared,
        .z_plus_1_face = current + n_squared,
        .next_z_face = next,
        .source_face = source,
    };

    // Middle slices (0<z<n-1)
    for (unsigned int z = 1; z < n_minus_1; z++) {
        const unsigned int z_offset = z*n_squared;
        slices[z] = (slice_t){
            .z_minus_1_face = current + z_offset - n_squared,
            .z_face = current + z_offset,
            .z_plus_1_face = current + z_offset + n_squared,
            .next_z_face = next + z_offset,
            .source_face = source + z_offset,
        };
    }

    // Last slice (boundary case)
    const unsigned int z_offset = (n-1)*n_squared;
    slices[n_minus_1] = (slice_t){
        .z_minus_1_face = current + z_offset - n_squared,
        .z_plus_1_face = current + z_offset - n_squared,
        .z_face = current + z_offset,
        .next_z_face = next + z_offset,
        .source_face = source + z_offset,
    };

    slice_t* slice_start = slices;
    const unsigned int slice_count = n / NUM_THREADS;
    unsigned int remainder = n % NUM_THREADS;

    pthread_t threads[NUM_THREADS - 1];
    thread_work_block_t thread_work_blocks[NUM_THREADS - 1];

    // Create NUM_THREADS-1 threads as the current thread becomes the last thread
    for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
        thread_work_blocks[i] = (thread_work_block_t){
            .slice_start = slice_start,
            .thread_done_iteration_semaphore = thread_done_iteration_semaphores + i,
            .pointer_swap_done_semaphore = pointer_swap_done_semaphores + i
        };
        if (remainder > 0) {
            // The first few threads are created with one extra slice until the remainder is 0
            thread_work_blocks[i].slice_count = slice_count + 1;
            pthread_create(threads + i, NULL, process_thread_work_block, thread_work_blocks + i);
            slice_start += slice_count + 1;
            remainder--;
        } else {
            thread_work_blocks[i].slice_count = slice_count;
            pthread_create(threads + i, NULL, process_thread_work_block, thread_work_blocks + i);
            slice_start += slice_count;
        }
    }

    // For all iterations except the last
    for (unsigned int iteration = 0; iteration < iterations - 1; iteration++) {
        for (slice_t* slice = slice_start; slice < slice_start + slice_count; slice++) { // scary for loop that increases a pointer
            process_slice(*slice);
        }

        for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
            sem_wait(thread_done_iteration_semaphores + i);
        }


        #ifdef USE_MEMCPY
            our_memcpy(current, next, n_cubed_bytes);
        #else
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
        #endif

        for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
            sem_post(pointer_swap_done_semaphores + i);
        }
    }

    // Last iteration
    for (slice_t* slice = slice_start; slice < slice_start + slice_count; slice++) { // scary for loop that increases a pointer
        process_slice(*slice);
    }

    for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
        // TODO test using the semaphore instead
        pthread_join(threads[i], NULL);
    }
}
#endif


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
    source = (data_t*)calloc(n_cubed_bytes * 2, 1);
    current = (data_t*)(((uint8_t*)source) + n_cubed_bytes);
    next = (data_t*)malloc(n_cubed_bytes + n * sizeof(slice_t));
    slices = (slice_t*)(((uint8_t*)next) + n_cubed_bytes);
    
    // Multiple source by DELTA ^ 2 outside of loops
    source[(n*n*n) / 2] = DELTA * DELTA * 1;

    run();

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

    // Free source pointer
    free(source);

    #ifdef USE_MEMCPY
        free(next);
    #else
        // Since next and current are swapped, free the original next pointer (which is swapped to current for an even number of iterations)
        if (iterations & 1) {
            free(next);
        } else {
            free(current);
        }
    #endif

    #ifdef PRINT_TIME
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    printf("%ld ", end_time.tv_nsec - start_time.tv_nsec);
    printf("%ld", end_time.tv_sec - start_time.tv_sec);
    #endif

    return EXIT_SUCCESS;
}