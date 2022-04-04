#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


/**
 * threads.c
 * Example of using pthread to create a set of worker threads.
 *
 * BUILDING:
 * g++ -o threads threads.c -lpthread
 *
 * TODO:
 * 1 - Read through this example, understand how threads work.
 * 2 - Change NUM_THREADS to see how the program finishes faster.
 * 2 - Consider how this could be applied to the Poisson solver.
 */

#define NUM_THREADS     4


typedef struct {
    int thread_id;      // unique id of the worker thread
    int start;          // start index of the worker thread
    int end;            // end index of the worker thread
} WorkerArgs;


void* worker(void* pargs) {
    WorkerArgs* args = (WorkerArgs*)pargs;

    for (int i = args->start; i < args->end; ++i) {
        // print out some indentation
        for (int j = 0; j < args->thread_id; ++j) {
            putchar('\t');
        }
        printf("%i\n", i);
        // delay a random amount up to 1 second
        usleep(rand() % 1000000);
    }

    return NULL;
}


int main(int argc, char** argv) {
    int range = 100;

    // Storage for the thread handles and arguments
    // will exist for the entire lifetime of the program.
    pthread_t threads[NUM_THREADS];
    WorkerArgs args[NUM_THREADS];

    // Launch each of the new worker threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        // Fill in the arguments to the worker
        args[i].thread_id = i;
        args[i].start = (range * i) / NUM_THREADS;
        args[i].end = (range * (i + 1)) / NUM_THREADS;

        // Create the worker thread
        if (pthread_create(&threads[i], NULL, &worker, &args[i]) != 0) {
            fprintf(stderr, "Error creating worker thread!\n");
            return EXIT_FAILURE;
        }
    }

    // Wait for all the threads to finish using join()
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    return EXIT_SUCCESS;
}
