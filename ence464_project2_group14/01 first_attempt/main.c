#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#ifdef PRINT_TIME
static struct timespec start_time;
static struct timespec end_time;
#endif

/**
 * poisson.c
 * Implementation of a Poisson solver with Neumann boundary conditions.
 *
 * This template handles the basic program launch, argument parsing, and memory
 * allocation required to implement the solver *at its most basic level*. You
 * will likely need to allocate more memory, add threading support, account for
 * cache locality, etc...
 *
 * BUILDING:
 * If using c++:
 * g++ -o poisson poisson.c -lpthread
 * If using vanilla c:
 * gcc -o poisson poisson.c -lpthread 
 *
 * [note: linking pthread isn't strictly needed until you add your
 *        multithreading code]
 *
 * TODO:
 * 1 - Read through this example, understand what it does and what it gives you
 *     to work with.
 * 2 - Implement the basic algorithm and get a correct output.
 * 3 - Add a timer to track how long your execution takes.
 * 4 - Profile your solution and identify weaknesses.
 * 5 - Improve it!
 * 6 - Remember that this is now *your* code and *you* should modify it however
 *     needed to solve the assignment.
 *
 * See the lab notes for a guide on profiling and an introduction to
 * multithreading (see also threads.c which is reference by the lab notes).
 */


// Global flag
// set to true when operating in debug mode to enable verbose logging
static bool debug = false;


/**
 * @brief Solve Poissons equation for a given cube with Neumann boundary
 * conditions on all sides.
 *
 * @param n             The edge length of the cube. n^3 number of elements.
 * @param source        Pointer to the source term cube, a.k.a. forcing function.
 * @param iterations    Number of iterations to solve with.
 * @param threads       Number of threads to use for solving.
 * @param delta         Grid spacing.
 * @return double*      Solution to Poissons equation. Caller must free().
 */
double* poisson_neumann(int n, double *source, int iterations, int threads, float delta) {
    if (debug) {
        printf("Starting solver with:\n"
               "n = %i\n"
               "iterations = %i\n"
               "threads = %i\n"
               "delta = %f\n",
               n, iterations, threads, delta);
    }

    // Allocate some buffers to calculate the solution in
    // Each buffer stores cube in one time step
    double *curr = (double*)calloc(n * n * n, sizeof(double));
    double *next = (double*)calloc(n * n * n, sizeof(double));

    // Ensure we haven't run out of memory
    if (curr == NULL || next == NULL) {
        fprintf(stderr, "Error: ran out of memory when trying to allocate %i sized cube\n", n);
        exit(EXIT_FAILURE);
    }

    // Define array size
    size_t cube_mem_size = (size_t)n * n * n * sizeof(double);

    for (unsigned int iter = 0; iter < iterations; iter++) {  // For each iteration
        for (unsigned int x = 0; x < n; x++) {  // For each x node (assuming cube)
            for (unsigned int y = 0; y < n; y++) {  // For each y node (assuming cube)
                for (unsigned int z = 0; z < n; z++) {  // For each z node (assuming cube)

                    // Stores solved potential for this node 
                    double node_potential = 0;

                    // Handle x dimension 
                    if (x < n - 1) { // X_(x+1) component
                        node_potential += curr[((z * n) + y)*n + (x + 1)];
                    } else {
                        node_potential += curr[((z * n) + y)*n + (x - 1)]; // apply Neumann BC
                    }

                    if (x > 0) { // X_(x-1) component
                        node_potential += curr[((z * n) + y)*n + (x - 1)];
                    } else {
                        node_potential += curr[((z * n) + y)*n + (x + 1)]; // apply Neumann BC
                    }

                    // Handle y dimension
                    if (y < n - 1) {
                        node_potential += curr[((z * n) + (y + 1))*n + x];
                    } else {
                        node_potential += curr[((z * n) + (y - 1))*n + x]; // apply Neumann BC
                    }

                    if (y > 0) {
                        node_potential += curr[((z * n) + (y - 1))*n + x];
                    } else {
                        node_potential += curr[((z * n) + (y + 1))*n + x]; // apply Neumann BC
                    }

                    // Handle z dimension 
                    if (z < n - 1) {
                        node_potential += curr[(((z+1) * n) + (y))*n + x];
                    } else {
                        node_potential += curr[(((z-1) * n) + (y))*n + x]; // apply Neumann BC
                    }

                    if (z > 0) {
                        node_potential += curr[(((z-1) * n) + y)*n + x];
                    } else {
                        node_potential += curr[(((z+1) * n) + y)*n + x]; // apply Neumann BC
                    }

                    // Perform Poisson discrete update step:
                    node_potential -= delta*delta*source[((z * n) + y)*n + (x)];
                    node_potential /= 6;

                    next[((z * n) + y)*n + (x)] = node_potential;
                }
            }
        }
        memcpy(curr, next, cube_mem_size);
    }

    // Free one of the buffers and return the correct answer in the other.
    // The caller is now responsible for free'ing the returned pointer.
    free(next);

    if (debug) {
        printf("Finished solving.\n");
    }

    return curr;
}




/// Solve Poisson's equation for a rectangular box with Dirichlet
/// boundary conditions on each face.
/// \param source is a pointer to a flattened 3-D array for the source function
/// \param potential is a pointer to a flattened 3-D array for the calculated potential
/// \param Vbound is the potential on the boundary
/// \param xsize is the number of elements in the x-direction
/// \param ysize is the number of elements in the y-direction
/// \param zsize is the number of elements in the z-direction
/// \param delta is the voxel spacing in all directions
/// \param numiters is the number of iterations to perform
/// \param numcores is the number of CPU cores to use.  If 0, an optimal number is chosen
void poisson_dirichlet (double * __restrict__ source,
                        double * __restrict__ potential,
                        double Vbound,
                        unsigned int xsize, unsigned int ysize, unsigned int zsize, double delta,
                        unsigned int numiters, unsigned int numcores)
{
    // source[i, j, k] is accessed with source[((k * ysize) + j) * xsize + i]
    // potential[i, j, k] is accessed with potential[((k * ysize) + j) * xsize + i]    
    size_t size = (size_t)ysize * zsize * xsize * sizeof(double);
	double *input = (double *)malloc(size);
	if (!input) {
		fprintf(stderr, "malloc failure\n");
		return;
	}
	memcpy(input, source, size);
	for (unsigned int iter = 0; iter < numiters; iter++) {
		for (unsigned int x = 0; x < xsize; x++) {
			for (unsigned int z = 0; z < zsize; z++) {
				for (unsigned int y = 0; y < ysize; y++) {
					double res = 0;

					if (x < xsize - 1)
						res += input[((z * ysize) + y) * xsize + (x + 1)];
					else
						res += Vbound;
					if (x > 0)
						res += input[((z * ysize) + y) * xsize + (x - 1)];
					else
						res += Vbound;

					if (y < ysize - 1)
						res += input[((z * ysize) + (y + 1)) * xsize + x];
					else
						res += Vbound;
					if (y > 0)
						res += input[((z * ysize) + (y - 1)) * xsize + x];
					else
						res += Vbound;

					if (z < zsize - 1)
						res += input[(((z + 1) * ysize) + y) * xsize + x];
					else
						res += Vbound;
					if (z > 0)
						res += input[(((z - 1) * ysize) + y) * xsize + x];
					else
						res += Vbound;

					res -= delta * delta * source[((z * ysize) + y) * xsize + x];

					res /= 6;

					potential[((z * ysize) + y) * xsize + x] = res;
				}
			}
		}
		memcpy(input, potential, size);
	}
	free(input);
}


int main(int argc, char **argv) {
    #ifdef PRINT_TIME
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    #endif

    // default settings for solver
    int iterations = 10;
    int n = 5;
    int threads = 1;
    float delta = 1;

    // // parse the command line arguments
    // for (int i = 1; i < argc; ++i) {
    //     if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
    //         printf("usage: poisson [-n size] [-i iterations] [-t threads] [--debug]\n");
    //         return EXIT_SUCCESS;
    //     }

    //     if (strcmp(argv[i], "-n") == 0) {
    //         if (i == argc - 1) {
    //             fprintf(stderr, "Error: expected size after -n!\n");
    //             return EXIT_FAILURE;
    //         }

    //         n = atoi(argv[++i]);
    //     }

    //     if (strcmp(argv[i], "-i") == 0) {
    //         if (i == argc - 1) {
    //             fprintf(stderr, "Error: expected iterations after -i!\n");
    //             return EXIT_FAILURE;
    //         }

    //         iterations = atoi(argv[++i]);
    //     }

    //     if (strcmp(argv[i], "-t") == 0) {
    //         if (i == argc - 1) {
    //             fprintf(stderr, "Error: expected threads after -t!\n");
    //             return EXIT_FAILURE;
    //         }

    //         threads = atoi(argv[++i]);
    //     }

    //     if (strcmp(argv[i], "--debug") == 0) {
    //         debug = true;
    //     }
    // }

    // // ensure we have an odd sized cube
    // if (n % 2 == 0) {
    //     fprintf(stderr, "Error: n should be an odd number!\n");
    //     return EXIT_FAILURE;
    // }

    if (argc != 3) {
        fprintf(stderr, "[Error]: wrong arguments must be <n> <iterations>\n");
        return EXIT_FAILURE;
    }

    n = atoi(argv[1]);
    iterations = atoi(argv[2]);

    // Create a source term with a single point in the centre
    double *source = (double*)calloc(n * n * n, sizeof(double));
    if (source == NULL) {
        fprintf(stderr, "Error: failed to allocated source term (n=%i)\n", n);
        return EXIT_FAILURE;
    }

    source[(n * n * n) / 2] = 1;

    // Calculate the resulting field with Neumann conditions
    double *result = poisson_neumann(n, source, iterations, threads, delta);

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
