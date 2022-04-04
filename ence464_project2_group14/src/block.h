#include <stdlib.h>

typedef struct {
    double** neg_x;
    double** pos_x;
    double** neg_y;
    double** pos_y;
    double** neg_z;
    double** pos_z;
} block_faces_t;


// void process_block (
//     const unsigned int iterations,
//     double * __restrict__ source,
//     double * __restrict__ current,
//     double * __restrict__ next,
//     const unsigned int block_size,
//     const unsigned int block_size_minus_1,
//     const unsigned int block_size_squared,
//     const size_t block_size_bytes
// );