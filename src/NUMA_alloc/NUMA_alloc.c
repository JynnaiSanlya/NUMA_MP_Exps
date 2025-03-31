#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <numaif.h>
#include <unistd.h>
#include <omp.h>
#include <sched.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <alloc_size>\n", argv[0]);
        return EXIT_FAILURE;
    }
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int *ptr = (int *)malloc(atoi(argv[1])*sizeof(int)); // 分配内存
        sleep(3);
        if (!ptr) {
            printf("Thread %d: malloc failed\n", thread_id);
        }
        else{
            *ptr = 42; // 触发内存分配

            int status;
            int ret = move_pages(0, 1, (void **)&ptr, NULL, &status, 0);
            if (ret == 0) {
                printf("Thread %d: Memory address %p is on NUMA node %d\n", thread_id, ptr, status);
            } else {
                printf("Thread %d: move_pages failed\n", thread_id);
            }

            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);

            free(ptr);
        }
        sleep(3);
    }
    return 0;
}
