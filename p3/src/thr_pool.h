#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <time.h>
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

/*
 * Function: threadPoolInit()
 * Initialize the thread pool with 'numThreads' threads
 */
threadPool* threadPoolInit(int numThreads);

/*
 * Function: threadPoolEnqueue()
 * Add task to the task queue in the thread pool pointed by thrPool
 */
int threadPoolEnqueue(threadPool* thrPool, void (*funcPtr)(void*), void* arg);

/*
 * Function: threadPoolWait()
 * Wait for all the tasks in the thread pool
 pointed by thrPool to be finised
 */
void threadPoolWait(threadPool* thrPool);

/*
 * Function: signalThreadFunc()
 * Signal all active threads when there are available tasks
 */
void* signalThreadFunc(void* arg);

/*
 * Function: writeLog()
 * Write log about the response information including
 response time, client IP address and client port number
 */
int writeLog(int threadId, char* ipAddr, int portNum);

#endif  /*__THREAD_POOL_H__*/
