#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <numa.h>
#include <numaif.h>
#include <omp.h>
#include <time.h>

#define MATRIX_SIZE 8192
#define NUM_THREADS 6
int node;
int (*matrixA)[MATRIX_SIZE];
int (*matrixB)[MATRIX_SIZE];
int (*matrixC)[MATRIX_SIZE];

void allocate_on_numa_node() {
    size_t size = MATRIX_SIZE * MATRIX_SIZE * sizeof(int);

    // matrixA = (int (*)[MATRIX_SIZE])numa_alloc_interleaved(size);
    // matrixB = (int (*)[MATRIX_SIZE])numa_alloc_interleaved(size);
    // matrixC = (int (*)[MATRIX_SIZE])numa_alloc_interleaved(size);
    if(node == 0 || node == 1){
        matrixA = (int (*)[MATRIX_SIZE])numa_alloc_onnode(size,node);
        matrixB = (int (*)[MATRIX_SIZE])numa_alloc_onnode(size,node);
        matrixC = (int (*)[MATRIX_SIZE])numa_alloc_onnode(size,node);
    } else{
        matrixA = (int (*)[MATRIX_SIZE])numa_alloc_interleaved(size);
        matrixB = (int (*)[MATRIX_SIZE])numa_alloc_interleaved(size);
        matrixC = (int (*)[MATRIX_SIZE])numa_alloc_interleaved(size);
    }

    if (!matrixA || !matrixB || !matrixC) {
        fprintf(stderr, "NUMA allocation failed.\n");
        exit(EXIT_FAILURE);
    }
}

void free_numa_memory() {
    numa_free(matrixA, MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    numa_free(matrixB, MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    numa_free(matrixC, MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
}

void initialize_matrices() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrixA[i][j] = rand() % 100;
            matrixB[i][j] = rand() % 100;
        }
    }
}

void add_matrices() {
    #pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrixC[i][j] = matrixA[i][j] + matrixB[i][j];
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Mem Alloc Way>\n0 for node0, 1 for node1, others for interleaved.", argv[0]);
        return EXIT_FAILURE;
    }

    node = atoi(argv[1]);
    if (numa_available() == -1) {
        fprintf(stderr, "NUMA is not available on this system.\n");
        return EXIT_FAILURE;
    }

    int target_node = 0; // Change to 1 if you want to use NUMA node 1
    allocate_on_numa_node();
    
    initialize_matrices();
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for(int i = 0; i < 1000;++i)
        add_matrices();

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + 
                              (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Total time for 1000 iterations: %.3f seconds\n", elapsed_time);

    free_numa_memory();
    return 0;
}
