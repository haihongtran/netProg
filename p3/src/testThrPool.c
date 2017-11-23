#include "thr_pool.h"

#define THR_NUM 10

void task1() {
    printf("Thread #%u is working on task 1\n", (int)pthread_self());
}


void task2() {
    printf("Thread #%u is working on task 2\n", (int)pthread_self());
}


int main() {
    int i;

    printf("Creating a thread pool with %d threads\n", THR_NUM);
    threadPool* thrPool = threadPoolInit(THR_NUM);

    printf("Adding 100 tasks to the thread pool\n");
    for (i = 0; i < 50; i++) {
        threadPoolEnqueue(thrPool, (void*)task1, NULL);
        threadPoolEnqueue(thrPool, (void*)task2, NULL);
    }

    /* Wait for all tasks to be finished */
    threadPoolWait(thrPool);

    return 0;
}
