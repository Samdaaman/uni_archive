#include "block2.h"
#include <stdlib.h>
#include <string.h>


void process_block (
    const unsigned int iterations,
    double * __restrict__ source,
    double * __restrict__ current,
    double ** __restrict__ current_pointers,
    double * next,
    const unsigned int block_size,
    const unsigned int block_size_squared,
    const size_t block_size_bytes,
) {
    for (unsigned int i = 0; i < iterations; i++) {
        for (unsigned int z = 0; z < block_size; z++) {
            const unsigned int z_offset = z * block_size;
            const unsigned int z_offset_padded = z_offset + block_size; // (z + 1) * block_size
            for (unsigned int y = 0; y < block_size; y++) {
                const unsigned int z_and_y_offset = (z_offset + y) * block_size;
                const unsigned int z_and_y_offset_padded = (z_offset_padded + y + 1) * block_size;
                for (unsigned int x = 0; x < block_size; x++) {
                    const unsigned int central_node_index = z_and_y_offset + x;
                    const unsigned int central_node_index_padded = z_and_y_offset_padded + x + 1;

                    next[central_node_index] = 1 / 6 * ( \
                        *(current_pointers[central_node_index_padded + 1]) + \
                        *(current_pointers[central_node_index_padded - 1]) + \
                        *(current_pointers[central_node_index_padded + block_size]) + \
                        *(current_pointers[central_node_index_padded - block_size]) + \
                        *(current_pointers[central_node_index_padded + block_size_squared]) + \
                        *(current_pointers[central_node_index_padded - block_size_squared]) - \
                        source[central_node_index] \
                    );
                }
            }
        }

        // TODO when there is threads, wait here for syncronisation
        memcpy(current, next, block_size_bytes);
    }
}