#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DELTA 1.0


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
static float* __restrict__ source;
static float* __restrict__ current;
static float* __restrict__ next;
static sem_t thread_done_iteration_semaphores[NUM_THREADS - 1];
static sem_t memcpy_done_semaphores[NUM_THREADS - 1];


typedef struct {
    float* __restrict__ z_minus_1_face;
    float* __restrict__ z_face;
    float* __restrict__ z_plus_1_face;
    float* __restrict__ source_face;
    float* __restrict__ next_z_face;
} slice_t;

typedef struct {
    slice_t* slice_start;
    unsigned int slice_count;
    sem_t* thread_done_iteration_semaphore;
    sem_t* memcpy_done_semaphore;
} thread_work_block_t;



void process_slice(slice_t slice) {
    // TODO time analysis of array access order (ie access z_minus_1_face first instead of z_face[x + 1]) as it is first in the array


    // x=0 & y=0 corner
    slice.next_z_face[0] = 1 / 6.0 * ( \
        2 * slice.z_face[1] + \
        2 * slice.z_face[n] + \
        slice.z_plus_1_face[0] + \
        slice.z_minus_1_face[0] - \
        slice.source_face[0] \
    );

    // y=0 & 0<x<n_minus_1 edge
    for (unsigned int x = 1; x < n_minus_1; x++) {
        slice.next_z_face[x] = 1 / 6.0 * ( \
            slice.z_face[x + 1] + \
            slice.z_face[x - 1] + \
            2 * slice.z_face[x + n] + \
            slice.z_plus_1_face[x] + \
            slice.z_minus_1_face[x] - \
            slice.source_face[x] \
        );
    }

    // y=0 & x=n_minus_1 corner
    slice.next_z_face[n_minus_1] = 1 / 6.0 * ( \
        2 * slice.z_face[n_minus_1 - 1] + \
        2 * slice.z_face[n_minus_1 + n] + \
        slice.z_plus_1_face[n_minus_1] + \
        slice.z_minus_1_face[n_minus_1] - \
        slice.source_face[n_minus_1] \
    );

    for (unsigned int y = 1; y < n_minus_1; y++) {
        // 0<y<n_minus_1 & x=0 edge
        slice.next_z_face[y*n] = 1 / 6.0 * ( \
            2*slice.z_face[y*n + 1] + \
            slice.z_face[y*n + n] + \
            slice.z_face[y*n - n] + \
            slice.z_plus_1_face[y*n] + \
            slice.z_minus_1_face[y*n] - \
            slice.source_face[y*n] \
        );

        // 0<y<n_minus_1 & 0<x<n_minus_1 (middle section with no BCs)
        for (unsigned int x = 1; x < n_minus_1; x++) {
            slice.next_z_face[y*n + x] = 1 / 6.0 * ( \
                slice.z_face[y*n + x + 1] + \
                slice.z_face[y*n + x - 1] + \
                slice.z_face[y*n + x + n] + \
                slice.z_face[y*n + x - n] + \
                slice.z_plus_1_face[y*n + x] + \
                slice.z_minus_1_face[y*n + x] - \
                slice.source_face[y*n + x] \
            );
        }

        // 0<y<n_minus_1 & x=n_minus_1 edge
        slice.next_z_face[y*n + n_minus_1] = 1 / 6.0 * ( \
            2*slice.z_face[y*n + n_minus_1 - 1] + \
            slice.z_face[y*n + n + n_minus_1] + \
            slice.z_face[y*n - n + n_minus_1] + \
            slice.z_plus_1_face[y*n + n_minus_1] + \
            slice.z_minus_1_face[y*n + n_minus_1] - \
            slice.source_face[y*n + n_minus_1] \
        );
    }

    // y=n_minus_1 & x=0 corner
    slice.next_z_face[n_minus_1 * n] = 1 / 6.0 * ( \
        2 * slice.z_face[n_minus_1 * n + 1] + \
        2 * slice.z_face[n_minus_1 * n - n] + \
        slice.z_plus_1_face[n_minus_1 * n] + \
        slice.z_minus_1_face[n_minus_1 * n] - \
        slice.source_face[n_minus_1 * n] \
    );

    // y=n_minus_1 & 0<x<n_minus_1 edge
    for (unsigned int x = 1; x < n_minus_1; x++) {
        slice.next_z_face[n_minus_1 * n + x] = 1 / 6.0 * ( \
            slice.z_face[n_minus_1 * n + x + 1] + \
            slice.z_face[n_minus_1 * n + x - 1] + \
            2 * slice.z_face[n_minus_1 * n - n + x] + \
            slice.z_plus_1_face[n_minus_1 * n + x] + \
            slice.z_minus_1_face[n_minus_1 * n + x] - \
            slice.source_face[x] \
        );
    }

    // y=n_minus_1 & x=n_minus_1 corner
    slice.next_z_face[n_minus_1 * n + n_minus_1] = 1 / 6.0 * ( \
        2 * slice.z_face[n_minus_1 * n + n_minus_1 - 1] + \
        2 * slice.z_face[n_minus_1 * n - n + n_minus_1] + \
        slice.z_plus_1_face[n_minus_1 * n + n_minus_1] + \
        slice.z_minus_1_face[n_minus_1 * n + n_minus_1] - \
        slice.source_face[n_minus_1 * n + n_minus_1] \
    );
}


void* process_thread_work_block(void* thread_work_block) {
    // For all iterations except the last
    for (unsigned int iteration = 0; iteration < iterations - 1; iteration++) {
        for (slice_t* slice = ((thread_work_block_t*)thread_work_block)->slice_start; slice < ((thread_work_block_t*)thread_work_block)->slice_start + ((thread_work_block_t*)thread_work_block)->slice_count; slice++) { // scary for-loop that increases a pointer
            process_slice(*slice);
        }
        sem_post(((thread_work_block_t*)thread_work_block)->thread_done_iteration_semaphore);
        sem_wait(((thread_work_block_t*)thread_work_block)->memcpy_done_semaphore);
    }

    // Last iterations
    for (slice_t* slice = ((thread_work_block_t*)thread_work_block)->slice_start; slice < ((thread_work_block_t*)thread_work_block)->slice_start + ((thread_work_block_t*)thread_work_block)->slice_count; slice++) { // scary for-loop that increases a pointer
        process_slice(*slice);
    }
    // Don't worry about any semaphores as the main thread joins all the threads to make sure they terminate
}


void run_threaded(void) {
    // TODO optimise callocs with one call and union type
    current = (float*)calloc(n_cubed_bytes, 1);
    
    // TODO malloc as this array is written before it's read
    next = (float*)calloc(n_cubed_bytes, 1);

    slice_t* slices = (slice_t*)calloc(n, sizeof(slice_t));
    
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
            .memcpy_done_semaphore = memcpy_done_semaphores + i
        };
        if (remainder > 0) {
            // The first few threads are created with one extra slice until the remainder is 0
            thread_work_blocks[i].slice_count = slice_count + 1;
            pthread_create(threads + i, NULL, process_thread_work_block, thread_work_blocks + i);
            slice_start += slice_count + 1;
            // fprintf(stderr, "Created thread %i with %i slices (remainder=%i)\n", i, thread_work_blocks[i].slice_count, remainder);
            remainder--;
        } else {
            thread_work_blocks[i].slice_count = slice_count;
            pthread_create(threads + i, NULL, process_thread_work_block, thread_work_blocks + i);
            slice_start += slice_count;
            // fprintf(stderr, "Created thread %i with %i slices\n", i, thread_work_blocks[i].slice_count);
        }
    }

    // For all iterations except the last
    for (unsigned int iteration = 0; iteration < iterations - 1; iteration++) {
        // fprintf(stderr, "iteration=%i\n", iteration);
        for (slice_t* slice = slice_start; slice < slice_start + slice_count; slice++) { // scary for loop that increases a pointer
            // fprintf(stderr, "Processing slice at %p\n", slice);
            process_slice(*slice);
        }

        for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
            // fprintf(stderr, "waiting for sem %i\n", i);
            sem_wait(thread_done_iteration_semaphores + i);
        }

        // fprintf(stderr, "doing memcpy\n");
        memcpy(current, next, n_cubed_bytes);
        // fprintf(stderr, "done memcpy\n");

        for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
            // fprintf(stderr, "posting sem %i\n", i);
            sem_post(memcpy_done_semaphores + i);
        }
    }

    // Last iteration
    // fprintf(stderr, "Last iteration\n");
    for (slice_t* slice = slice_start; slice < slice_start + slice_count; slice++) { // scary for loop that increases a pointer
        process_slice(*slice);
    }

    for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
        // fprintf(stderr, "pthread_join for thread %i\n", i);
        // TODO test using the semaphore instead
        pthread_join(threads[i], NULL);
    }
    // fprintf(stderr, "Done\n");
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
    n_cubed_bytes = n_squared * n * sizeof(float);
    iterations = atoi(argv[2]);

    for (unsigned int i = 0; i < NUM_THREADS - 1; i++) {
        sem_init(thread_done_iteration_semaphores + i, 0, 0);
        sem_init(memcpy_done_semaphores + i, 0, 0);
    }

    #ifdef PRINT_OUTPUT
    fprintf(stderr, "[Debug]: Running %ix%ix%i with %i iterations\n", n, n, n, iterations);
    #endif

    // Create a source term with a single point in the centre
    source = (float*)calloc(n * n * n, sizeof(float));
    source[(n * n * n) / 2] = DELTA * DELTA * 1;

    run_threaded();

    // Old "reference" way
    // // Print out the middle slice (ie Z cross-section) of the cube for validation
    // for (int x = 0; x < n; ++x) {
    //     for (int y = 0; y < n; ++y) {
    //         printf("%0.5f ", result[((n/2) * n + y) * n + x]);
    //     }
    //     printf("\n");
    // }

    // New "reference2" way
    // Print out the 1st, 2nd, 3rd, 4th, middle-1, middle, middle+1, 2nd last, last slices
    #ifdef PRINT_OUTPUT
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

    free(source);
    free(next);

    #ifdef PRINT_TIME
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    printf("%ld ", end_time.tv_nsec - start_time.tv_nsec);
    printf("%ld", end_time.tv_sec - start_time.tv_sec);
    #endif

    return EXIT_SUCCESS;
}