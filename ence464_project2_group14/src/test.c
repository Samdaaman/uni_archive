#include <stdlib.h>
#include <stdio.h>
#include <string.h>

double* test_single_threaded(const unsigned int n, double* source) {
    const unsigned int block_size = n;
    const unsigned int block_size_squared = block_size * block_size;
    const size_t block_size_bytes = block_size_squared * block_size * sizeof(double);

    double *dest = (double*)calloc(block_size_bytes, 1);
    // double *current = (double*)calloc(block_size_bytes, 1);
    double *current = source;
    double **current_pointers = (double**)calloc((block_size+2)*(block_size+2)*(block_size+2), sizeof(double*));

    // Map the middle section
    for (unsigned int z = 0; z < block_size; z++) {
        for (unsigned int y = 0; y < block_size; y++) {
            for (unsigned int x = 0; x < block_size; x++) {
                const unsigned int central_node_index = (z * block_size + y) * block_size + x;
                const unsigned int central_node_index_padded = ((z+1) * (block_size+2) + y+1) * (block_size+2) + x + 1;
                current_pointers[central_node_index_padded] = &current[central_node_index];
            }
        }
    }

    // Map the boundary's
    // x boundary
    for (unsigned int z = 0; z < block_size; z++) {
        for (unsigned int y = 0; y < block_size; y++) {
            // Negative x boundary
            current_pointers[((z+1) * (block_size+2) + y+1) * (block_size+2)] = &current[(z * block_size + y) * block_size + 1]; // reference x+1

            // Positive x boundary
            current_pointers[((z+1) * (block_size+2) + y+1) * (block_size+2) + block_size + 1] = &current[(z * block_size + y) * block_size + block_size - 2]; // reference x-1
        }
    }

    // y boundary
    for (unsigned int z = 0; z < block_size; z++) {
        for (unsigned int x = 0; x < block_size; x++) {
            // Negative y boundary
            current_pointers[(z+1) * (block_size+2) * (block_size+2) + x] = &current[z * block_size * block_size + block_size + x]; // reference y+1

            // Positive y boundary
            current_pointers[(z+1) * (block_size+2) * (block_size+2) + (block_size+2) * (block_size+2 - 1) + x] = &current[z * block_size * block_size + block_size * (block_size-1) + x]; // reference y-1
        }
    }

    // // z boundary
    // for (unsigned int y = 0; y < block_size; y++) {
    //     for (unsigned int x = 0; x < block_size; x++) {
    //         // Negative z boundary
    //         current_pointers[y * (block_size + 2) + x] = &current[y * block_size + x + block_size_squared]; // reference z+1

    //         // Positive z boundary
    //         current_pointers[(block_size+2) * (block_size+2) * (block_size+2 - 1) + y * (block_size+2) + x] = &current[block_size_squared*(block_size-1) + y*block_size + x]; // reference z-1
    //     }
    // }

    for (unsigned int z = 0; z < block_size + 2; z++) {
        printf("z=%i -----------------------------------------------\n", z);
        for (unsigned int y = 0; y < block_size + 2; y++) {
            for (unsigned int x = 0; x < block_size + 2; x++) {
                double * test_pointer = current_pointers[z*(block_size+2)*(block_size+2) + y*(block_size+2) + x];
                if (test_pointer != NULL) {
                    printf("*%03.0f* ", *test_pointer);
                } else {
                    printf("[N\\A] ");
                }
            }
            printf("\n");
        }
    }
}

int main() {
    const int n = 4;
    double *source = (double*)calloc(n * n * n, sizeof(double));

    for (unsigned int z = 0; z < n; z++) {
        for (unsigned int y = 0; y < n; y++) {
            for (unsigned int x = 0; x < n; x++) {
                source[z*n*n + y*n + x] = z*n*n + y*n + x;
            }
        }
    }
    test_single_threaded(n, source);
}