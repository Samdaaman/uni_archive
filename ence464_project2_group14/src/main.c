#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"


double* test_single_threaded(const unsigned int n, const unsigned int iterations, const float delta, double* source) {
    const unsigned int block_size = n;
    const unsigned int block_size_squared = block_size * block_size;
    const size_t block_size_bytes = block_size_squared * block_size * sizeof(double);

    double *dest = (double*)calloc(block_size_bytes, 1);
    double *current = (double*)calloc(block_size_bytes, 1);
    double **current_pointers = (double**)calloc((block_size+2)*(block_size+2)*(block_size+2), sizeof(double*));

    // Map the middle section
    for (unsigned int z = 0; z < block_size; z++) {
        const unsigned int z_offset = z * block_size;
        const unsigned int z_offset_padded = z_offset + block_size; // (z + 1) * block_size
        for (unsigned int y = 0; y < block_size; y++) {
            const unsigned int z_and_y_offset = (z_offset + y) * block_size;
            const unsigned int z_and_y_offset_padded = (z_offset_padded + y + 1) * block_size;
            for (unsigned int x = 0; x < block_size; x++) {
                const unsigned int central_node_index = z_and_y_offset + x;
                const unsigned int central_node_index_padded = z_and_y_offset_padded + x + 1;

                current_pointers[central_node_index_padded] = &current[central_node_index];
            }
        }
    }

    // Map the boundary's
    // x boundary
    for (unsigned int z = 0; z < block_size; z++) {
        for (unsigned int y = 0; y < block_size; y++) {
            // Negative x boundary
            current_pointers[(z * (block_size+2) + y) * (block_size+2)] = &current[(z * block_size + y) * block_size + 1]; // reference x+1

            // Positive x boundary
            current_pointers[(z * (block_size+2) + y) * (block_size+2) + block_size + 1] = &current[(z * block_size + y) * block_size + block_size - 1]; // reference x-1


    // y boundary
    for (unsigned int z = 0; z < block_size; z++) {
        for (unsigned int x = 0; x < block_size; x++) {
            // Negative y boundary
            current_pointers[z * (block_size+2) + x] = &current[z * block_size + block_size + x]; // reference y+1

            // Positive y boundary
            current_pointers[z * (block_size+2) + (block_size+2) * (block_size+2 - 1) + x] = &current[z * block_size - block_size + x]; // reference y-1
        }
    }

    // z boundary
    for (unsigned int y = 0; y < block_size; y++) {
        for (unsigned int x = 0; x < block_size; x++) {
            // Negative z boundary
            current_pointers[y * (block_size + 2) + x] = &current[y * block_size + x + block_size_squared]; // reference z+1

            // Positive z boundary
            current_pointers[(block_size+2) * (block_size+2) * (block_size+2 - 1) + y * (block_size+2) + x] = &current[block_size_squared*(block_size-1) + y*block_size + x]; // reference z-1
        }
    }

    process_block(iterations, source, current, dest, block_size, block_size_squared, block_size_bytes);
    return dest;
}


int main(int argc, char **argv) {

    // default settings for solver
    int iterations = 10;
    int n = 5;
    int threads = 1;
    float delta = 1;

    // parse the command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("usage: poisson [-n size] [-i iterations] [-t threads] [--debug]\n");
            return EXIT_SUCCESS;
        }

        if (strcmp(argv[i], "-n") == 0) {
            if (i == argc - 1) {
                fprintf(stderr, "Error: expected size after -n!\n");
                return EXIT_FAILURE;
            }

            n = atoi(argv[++i]);
        }

        if (strcmp(argv[i], "-i") == 0) {
            if (i == argc - 1) {
                fprintf(stderr, "Error: expected iterations after -i!\n");
                return EXIT_FAILURE;
            }

            iterations = atoi(argv[++i]);
        }

        if (strcmp(argv[i], "-t") == 0) {
            if (i == argc - 1) {
                fprintf(stderr, "Error: expected threads after -t!\n");
                return EXIT_FAILURE;
            }

            threads = atoi(argv[++i]);
        }
    }

    // ensure we have an odd sized cube
    if (n % 2 == 0) {
        fprintf(stderr, "Error: n should be an odd number!\n");
        return EXIT_FAILURE;
    }

    // Create a source term with a single point in the centre
    double *source = (double*)calloc(n * n * n, sizeof(double));
    if (source == NULL) {
        fprintf(stderr, "Error: failed to allocated source term (n=%i)\n", n);
        return EXIT_FAILURE;
    }

    source[(n * n * n) / 2] = 1;

    // Calculate the resulting field with Neumann conditions
    double *result = test_single_threaded(n, iterations, delta, source);

    // Print out the middle slice (ie Z cross-section) of the cube for validation
    for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
            printf("%0.5f ", result[((n/2) * n + y) * n + x]);
        }
        printf("\n");
    }

    free(source);
    free(result);

    return EXIT_SUCCESS;
}