#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "task_queue.h"

typedef struct thread {
    int id;
    pthread_t pthread;
    struct threadPool* thrPool;
} thread;

typedef struct threadPool {
    thread** threads;
    int numActiveThreads;
    pthread_mutex_t mutexThreadCnt;
    taskQueue taskQ;
} threadPool;

threadPool* threadPoolInit(int numThreads);
int threadPoolEnqueue(threadPool* thrPool, void (*funcPtr)(void*), void* arg);

#endif  /*__THREAD_POOL_H__*/
