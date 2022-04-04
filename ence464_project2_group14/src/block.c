#include "block.h"
#include <stdlib.h>
#include <string.h>


// When you try to make fast code...
// https://i.kym-cdn.com/photos/images/newsfeed/001/465/425/3fd.jpg
void process_block (
    const unsigned int iterations,
    double * __restrict__ source,
    double * __restrict__ current_middle,
    double * __restrict__ next_middle,
    block_faces_t faces,
    const unsigned int block_size,
    const unsigned int block_size_minus_1,
    const unsigned int block_size_squared,
    const size_t block_size_bytes,
) {
    for (unsigned int i = 0; i < iterations; i ++) {
        // ================================================================
        // Face edge cases (bondary conditions)
        // ================================================================
        
        // x boundary (x=0 and x=block_size-1)
        for (unsigned int z = 1; z < block_size_minus_1; z++) {
            const unsigned int z_offset = z * block_size;
            for (unsigned int y = 1; y < block_size_minus_1; y++) {
                const unsigned int central_node_index_negative = (z_offset + y) * block_size;  
                const unsigned int central_node_index_positive = central_node_index_negative + block_size_minus_1; // (z_offset + y) * block_size + block_size - 1

                // Postive x boundary (x=block_size-1)
                next_middle[central_node_index_positive] = 1 / 6 * ( \
                    /* current_middle[central_node_index_negative + 1] + */ \
                    *(faces.pos_x[z_offset + y]) /* Boundary condition (use face) */ + \ 
                    current_middle[central_node_index_positive - 1] +\ 
                    current_middle[central_node_index_positive + block_size] + \
                    current_middle[central_node_index_positive - block_size] + \
                    current_middle[central_node_index_positive + block_size_squared] + \
                    current_middle[central_node_index_positive - block_size_squared] - \
                    source[central_node_index_positive] \
                );
                
                // Negative x boundary (x=0)
                next_middle[central_node_index_negative] = 1 / 6 * ( \
                    current_middle[central_node_index_negative + 1] + \ 
                    /* current_middle[central_node_index_negative - 1] + */ \
                    *(faces.neg_x[z_offset + y]) /* Boundary condition (use face) */ + \ 
                    current_middle[central_node_index_negative + block_size] + \
                    current_middle[central_node_index_negative - block_size] + \
                    current_middle[central_node_index_negative + block_size_squared] + \
                    current_middle[central_node_index_negative - block_size_squared] - \
                    source[central_node_index_negative] \
                );
            }
        }

        // y boundary (y=0 and y=block_size-1)
        for (unsigned int z = 1; z < block_size_minus_1; z++) {
            const unsigned int z_offset = z * block_size;
            for (unsigned int x = 1; x < block_size_minus_1; x++) {
                const unsigned int central_node_index_negative = z_offset * block_size + x;  
                const unsigned int central_node_index_positive = central_node_index_negative + block_size_minus_1 * block_size;  // (z_offset + block_size - 1) * block_size + x

                // Positive y boundary
                next_middle[central_node_index_positive] = 1 / 6 * ( \
                    current_middle[central_node_index_positive + 1] + \
                    current_middle[central_node_index_positive - 1] + \
                    /* current_middle[central_node_index_positive + block_size] + */ \
                    *(faces.pos_y[z_offset + x]) + \
                    current_middle[central_node_index_positive - block_size] + \
                    current_middle[central_node_index_positive + block_size_squared] + \
                    current_middle[central_node_index_positive - block_size_squared] - \
                    source[central_node_index_positive] \
                );

                // Negative y boundary
                next_middle[central_node_index_negative] = 1 / 6 * ( \
                    current_middle[central_node_index_negative + 1] + \
                    current_middle[central_node_index_negative - 1] + \
                    current_middle[central_node_index_negative + block_size] + \
                    /* current_middle[central_node_index_negative - block_size] + */ \
                    *(faces.neg_y[z_offset + x]) + \
                    current_middle[central_node_index_negative + block_size_squared] + \
                    current_middle[central_node_index_negative - block_size_squared] - \
                    source[central_node_index_negative] \
                );
            }
        }

        // z boundary (z=0 and z=block_size-1)
        for (unsigned int y = 1; y < block_size_minus_1; y++) {
            const unsigned int y_offset = y * block_size;
            for (unsigned int x = 1; x < block_size_minus_1; x++) {
                const unsigned int central_node_index_negative = y_offset + x; // (0 * block_size + y) * block_size + x
                const unsigned int central_node_index_positive = central_node_index_negative + block_size * block_size * block_size_minus_1;  // ((block_size - 1) * block_size + y) * block_size + x

                // Positive z boundary
                next_middle[central_node_index_positive] = 1 / 6 * ( \
                    current_middle[central_node_index_positive + 1] + \
                    current_middle[central_node_index_positive - 1] + \
                    current_middle[central_node_index_positive + block_size] + \
                    current_middle[central_node_index_positive - block_size] + \
                    /* current_middle[central_node_index_positive + block_size_squared] + */ \
                    *(faces.pos_z[y_offset + x]) + \
                    current_middle[central_node_index_positive - block_size_squared] - \
                    source[central_node_index_positive] \
                );

                // Negative z boundary
                next_middle[central_node_index_negative] = 1 / 6 * ( \
                    current_middle[central_node_index_negative + 1] + \
                    current_middle[central_node_index_negative - 1] + \
                    current_middle[central_node_index_negative + block_size] + \
                    current_middle[central_node_index_negative - block_size] + \
                    current_middle[central_node_index_negative + block_size_squared] + \
                    /* current_middle[central_node_index_negative - block_size_squared] - */ \
                    *(faces.neg_z[y_offset + x]) - \
                    source[central_node_index_negative] \
                );
            }
        }

        // ================================================================
        // Corner edge cases (bondary conditions)
        // ================================================================
        // TODO corners


                
        // ================================================================
        // Normal cases (no boundary conditions)
        // ================================================================
        // TODO integrate with above code to avoid repeat loops
        for (unsigned int z = 1; z < block_size_minus_1; z++) {
            const unsigned int z_offset = z * block_size;
            for (unsigned int y = 1; y < block_size_minus_1; y++) {
                const unsigned int z_and_y_offset = (z_offset + y) * block_size;
                for (unsigned int x = 1; x < block_size_minus_1; x++) { 
                    const unsigned int central_node_index = z_and_y_offset + x;

                    next_middle[central_node_index] = 1 / 6 * ( \
                        current_middle[central_node_index + 1] + \
                        current_middle[central_node_index - 1] + \
                        current_middle[central_node_index + block_size] + \
                        current_middle[central_node_index - block_size] + \
                        current_middle[central_node_index + block_size_squared] + \
                        current_middle[central_node_index - block_size_squared] - \
                        source[central_node_index] \
                    );
                }
            }
        }
        memcpy(current_middle, next_middle, block_size_bytes);
    }
}

