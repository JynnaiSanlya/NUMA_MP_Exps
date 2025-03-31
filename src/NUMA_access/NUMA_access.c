#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <numa.h>
#include <numaif.h>
#include <sched.h>
#include <unistd.h>

int NUM_THREADS = 6;
int NUMA_NODE_0 = 0;
int NUMA_NODE_1 = 1;
int THREAD_NODE;
int MEM_NODE;
int MOVE = 0;

void *thread_func(void *arg) {
    int thread_id = *(int *)arg;
    free(arg);

    // Determine NUMA node for memory allocation
    // int local_numa_node = (thread_id < 3) ? NUMA_NODE_0 : NUMA_NODE_1;
    int local_numa_node = MEM_NODE;

    // Allocate memory for two input vectors and one result vector on the local NUMA node
    size_t vector_size = 2048 * 2048; // 1 MB for each vector
    int *vector_a = numa_alloc_onnode(vector_size, local_numa_node);
    int *vector_b = numa_alloc_onnode(vector_size, local_numa_node);
    int *vector_result = numa_alloc_onnode(vector_size, local_numa_node);

    if (!vector_a || !vector_b || !vector_result) {
        perror("numa_alloc_onnode");
        if (vector_a) numa_free(vector_a, vector_size);
        if (vector_b) numa_free(vector_b, vector_size);
        if (vector_result) numa_free(vector_result, vector_size);
        pthread_exit(NULL);
    }
    if(MOVE){
        // Move vectors to another NUMA node
        int target_numa_node = (local_numa_node == NUMA_NODE_0) ? NUMA_NODE_1 : NUMA_NODE_0;

        int *new_vector_a = numa_alloc_onnode(vector_size, target_numa_node);
        int *new_vector_b = numa_alloc_onnode(vector_size, target_numa_node);
        int *new_vector_result = numa_alloc_onnode(vector_size, target_numa_node);

        if (!new_vector_a || !new_vector_b || !new_vector_result) {
            perror("numa_alloc_onnode (target)");
            if (new_vector_a) numa_free(new_vector_a, vector_size);
            if (new_vector_b) numa_free(new_vector_b, vector_size);
            if (new_vector_result) numa_free(new_vector_result, vector_size);
            numa_free(vector_a, vector_size);
            numa_free(vector_b, vector_size);
            numa_free(vector_result, vector_size);
            pthread_exit(NULL);
        }

        // Copy data to the new NUMA node
        memcpy(new_vector_a, vector_a, vector_size);
        memcpy(new_vector_b, vector_b, vector_size);
        memcpy(new_vector_result, vector_result, vector_size);

        // Free memory on the original NUMA node
        numa_free(vector_a, vector_size);
        numa_free(vector_b, vector_size);
        numa_free(vector_result, vector_size);

        // Update pointers to point to the new NUMA node
        vector_a = new_vector_a;
        vector_b = new_vector_b;
        vector_result = new_vector_result;
    }
    // Initialize vectors
    for (size_t i = 0; i < vector_size / sizeof(int); i++) {
        vector_a[i] = i;
        vector_b[i] = i * 2;
    }

    // Perform vector addition
    for (size_t i = 0; i < vector_size / sizeof(int); i++) {
        vector_result[i] = vector_a[i] + vector_b[i];
    }

    // printf("Thread %d (NUMA node %d) performed vector addition on NUMA node %d\n",
    //        thread_id, local_numa_node, local_numa_node);

    // Free the allocated memory
    numa_free(vector_a, vector_size);
    numa_free(vector_b, vector_size);
    numa_free(vector_result, vector_size);

    pthread_exit(NULL);
}
int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <num_threads> <thread_numa_node> <mem_numa_node> <move_memory_available>\n", argv[0]);
        return EXIT_FAILURE;
    }

    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0 || NUM_THREADS > 6) {
        fprintf(stderr, "Error: Number of threads must be between 1 and 6.\n");
        return EXIT_FAILURE;
    }

    THREAD_NODE = atoi(argv[2]);
    if (THREAD_NODE != 0 && THREAD_NODE != 1) {
        fprintf(stderr, "Error: Thread NUMA node must be 0 or 1.\n");
        return EXIT_FAILURE;
    }

    MEM_NODE = atoi(argv[3]);
    if (MEM_NODE != 0 && MEM_NODE != 1) {
        fprintf(stderr, "Error: Memory NUMA node must be 0 or 1.\n");
        return EXIT_FAILURE;
    }if (numa_available() == -1) {
        fprintf(stderr, "NUMA is not available on this system.\n");
        return EXIT_FAILURE;
    }

    MOVE = atoi(argv[4]);
    if (MOVE < 0) {
        fprintf(stderr, "Error: 0 bans movement while positive number allows it.\n");
        return EXIT_FAILURE;
    }

    pthread_t threads[NUM_THREADS];
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int iter = 0; iter < 1000; iter++) {
        for (int i = 0; i < NUM_THREADS; i++) {
            int *thread_id = malloc(sizeof(int));
            *thread_id = i;

            // Create thread
            if (pthread_create(&threads[i], NULL, thread_func, thread_id) != 0) {
                perror("pthread_create");
                return EXIT_FAILURE;
            }

            // Bind thread to CPUs corresponding to NUMA node 0 (0-5)
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            if(!THREAD_NODE){
                for (int cpu = 0; cpu <= 5; cpu++) {
                    CPU_SET(cpu, &cpuset);
                }
            }
            else {
                for (int cpu = 6; cpu <= 11; cpu++) {
                    CPU_SET(cpu, &cpuset);
                }
            }

            if (pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset) != 0) {
                perror("pthread_setaffinity_np");
                return EXIT_FAILURE;
            }
        }

        // Wait for threads to finish
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = (end.tv_sec - start.tv_sec) + 
                          (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Total time for 1000 iterations: %.3f seconds\n", elapsed_time);
    return EXIT_SUCCESS;
}
