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
    int numWorkingThreads;
    pthread_mutex_t mutexThreadCnt;
    pthread_cond_t allThreadsIdle;
    taskQueue taskQ;
} threadPool;

/* Initialize thread pool*/
threadPool* threadPoolInit(int numThreads);

/* Add task to thread pool */
int threadPoolEnqueue(threadPool* thrPool, void (*funcPtr)(void*), void* arg);

/* Wait for all tasks to be complete */
void threadPoolWait(threadPool* thrPool);

#endif  /*__THREAD_POOL_H__*/
