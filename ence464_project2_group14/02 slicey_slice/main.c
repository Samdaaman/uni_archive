#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#ifdef PRINT_TIME
static struct timespec start_time;
static struct timespec end_time;
#endif


void process_face(
    double * __restrict__ z_minus_1_face,
    double * __restrict__ z_face,
    double * __restrict__ z_plus_1_face,
    double * __restrict__ source_face,
    double * __restrict__ next_z_face,
    const unsigned int n,
    const unsigned int n_minus_1,
    const float delta
) {
    // x=0 & y=0 corner
    next_z_face[0] = 1 / 6.0 * ( \
        2 * z_face[1] + \
        2 * z_face[n] + \
        z_plus_1_face[0] + \
        z_minus_1_face[0] - \
        source_face[0] \
    );

    // y=0 & 0<x<n_minus_1 edge
    for (unsigned int x = 1; x < n_minus_1; x++) {
        next_z_face[x] = 1 / 6.0 * ( \
            z_face[x + 1] + \
            z_face[x - 1] + \
            2 * z_face[x + n] + \
            z_plus_1_face[x] + \
            z_minus_1_face[x] - \
            source_face[x] \
        );
    }

    // y=0 & x=n_minus_1 corner
    next_z_face[n_minus_1] = 1 / 6.0 * ( \
        2 * z_face[n_minus_1 - 1] + \
        2 * z_face[n_minus_1 + n] + \
        z_plus_1_face[n_minus_1] + \
        z_minus_1_face[n_minus_1] - \
        source_face[n_minus_1] \
    );

    for (unsigned int y = 1; y < n_minus_1; y++) {
        // 0<y<n_minus_1 & x=0 edge
        next_z_face[y*n] = 1 / 6.0 * ( \
            2*z_face[y*n + 1] + \
            z_face[y*n + n] + \
            z_face[y*n - n] + \
            z_plus_1_face[y*n] + \
            z_minus_1_face[y*n] - \
            source_face[y*n] \
        );

        // 0<y<n_minus_1 & 0<x<n_minus_1 (middle section with no BCs)
        for (unsigned int x = 1; x < n_minus_1; x++) {
            next_z_face[y*n + x] = 1 / 6.0 * ( \
                z_face[y*n + x + 1] + \
                z_face[y*n + x - 1] + \
                z_face[y*n + x + n] + \
                z_face[y*n + x - n] + \
                z_plus_1_face[y*n + x] + \
                z_minus_1_face[y*n + x] - \
                source_face[y*n + x] \
            );
        }

        // 0<y<n_minus_1 & x=n_minus_1 edge
        next_z_face[y*n + n_minus_1] = 1 / 6.0 * ( \
            2*z_face[y*n + n_minus_1 - 1] + \
            z_face[y*n + n + n_minus_1] + \
            z_face[y*n - n + n_minus_1] + \
            z_plus_1_face[y*n + n_minus_1] + \
            z_minus_1_face[y*n + n_minus_1] - \
            source_face[y*n + n_minus_1] \
        );
    }

    // y=n_minus_1 & x=0 corner
    next_z_face[n_minus_1 * n] = 1 / 6.0 * ( \
        2 * z_face[n_minus_1 * n + 1] + \
        2 * z_face[n_minus_1 * n - n] + \
        z_plus_1_face[n_minus_1 * n] + \
        z_minus_1_face[n_minus_1 * n] - \
        source_face[n_minus_1 * n] \
    );

    // y=n_minus_1 & 0<x<n_minus_1 edge
    for (unsigned int x = 1; x < n_minus_1; x++) {
        next_z_face[n_minus_1 * n + x] = 1 / 6.0 * ( \
            z_face[n_minus_1 * n + x + 1] + \
            z_face[n_minus_1 * n + x - 1] + \
            2 * z_face[n_minus_1 * n - n + x] + \
            z_plus_1_face[n_minus_1 * n + x] + \
            z_minus_1_face[n_minus_1 * n + x] - \
            source_face[x] \
        );
    }

    // y=n_minus_1 & x=n_minus_1 corner
    next_z_face[n_minus_1 * n + n_minus_1] = 1 / 6.0 * ( \
        2 * z_face[n_minus_1 * n + n_minus_1 - 1] + \
        2 * z_face[n_minus_1 * n - n + n_minus_1] + \
        z_plus_1_face[n_minus_1 * n + n_minus_1] + \
        z_minus_1_face[n_minus_1 * n + n_minus_1] - \
        source_face[n_minus_1 * n + n_minus_1] \
    );
}


double* run_single_thread(
    const unsigned int n,
    const unsigned int iterations,
    const unsigned int delta,
    double * __restrict__ source
) {
    double *current = (double*)calloc(n * n * n, sizeof(double));
    double *next = (double*)calloc(n * n * n, sizeof(double));

    const unsigned int n_minus_1 = n-1;
    const unsigned int n_squared = n*n;
    const size_t size_bytes = n_squared*n*sizeof(double);

    for (unsigned int i = 0; i < iterations; i++) {
        // Edge case: First face
        process_face(
            current + n_squared,
            current,
            current + n_squared,
            source,
            next,
            n,
            n_minus_1,
            delta
        );

        // Middle faces
        for (unsigned int z = 1; z<n_minus_1; z++) {
            const unsigned int z_offset = z*n_squared;
            process_face(
                current + z_offset - n_squared,
                current + z_offset,
                current + z_offset + n_squared,
                source + z_offset,
                next + z_offset,
                n,
                n_minus_1,
                delta
            );
        }

        // Edge case: Last face
        const unsigned int z_offset = (n-1)*n_squared;
        process_face(
            current + z_offset - n_squared,
            current + z_offset,
            current + z_offset - n_squared,
            source + z_offset,
            next + z_offset,
            n,
            n_minus_1,
            delta
        );

        memcpy(current, next, size_bytes);
    }

    return current;
}


int main(int argc, char **argv) {
    #ifdef PRINT_TIME
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    #endif

    // default settings for solver
    int n = 7;
    int iterations = 300;
    float delta = 1;

    if (argc != 3) {
        fprintf(stderr, "[Error]: wrong arguments must be <n> <iterations>\n");
        return EXIT_FAILURE;
    }

    n = atoi(argv[1]);
    iterations = atoi(argv[2]);

    #ifndef PRINT_TIME
    fprintf(stderr, "[Debug]: Running %ix%ix%i with %i iterations\n", n, n, n, iterations);
    #endif

    // Create a source term with a single point in the centre
    double *source = (double*)calloc(n * n * n, sizeof(double));
    source[(n * n * n) / 2] = 1;

    double *result = run_single_thread(n, iterations, delta, source);

    free(source);
    free(result);

    #ifdef PRINT_TIME
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    printf("%ld ", end_time.tv_nsec - start_time.tv_nsec);
    printf("%ld", end_time.tv_sec - start_time.tv_sec);

    #else
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
    for (int z = 0; z < n; z++) {
        if (z < 4 || z == n/2-1 || z == n/2 || z == n/2+1 || z > n-3) {
            printf("--------------------z=%i--------------------\n", z);
            for (int x = 0; x < n; ++x) {
                for (int y = 0; y < n; ++y) {
                    printf("%0.5f ", result[(z * n + y) * n + x]);
                }
                printf("\n");
            }
        }
    }
    #endif

    return EXIT_SUCCESS;
}