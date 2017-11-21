#include "thr_pool.h"

void task1() {
    printf("Thread #%u working on task1\n", (int)pthread_self());
}


void task2() {
    printf("Thread #%u working on task2\n", (int)pthread_self());
}


int main() {
    puts("Making threadpool with 4 threads");
    threadPool* thrPool = threadPoolInit(4);

    puts("Adding 40 tasks to threadpool");
    int i;
    for (i=0; i<20; i++){
        threadPoolEnqueue(thrPool, (void*)task1, NULL);
        threadPoolEnqueue(thrPool, (void*)task2, NULL);
    };

    return 0;
}
