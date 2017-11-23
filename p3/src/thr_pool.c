#include "thr_pool.h"

/* Function prototypes */
int threadInit(threadPool* thrPool, thread** threadPtr, int id);
void* threadFunc(thread* threadPtr);

threadPool* threadPoolInit(int numThreads) {
    threadPool* thrPool = NULL;
    int i;
    /* Checking whether numThreads is valid */
    if ( numThreads <= 0 ) {
        printf("Cannot create a thread pool with thread number of %d.\n", numThreads);
        exit(-1);
    }
    /* Allocate memory for the thread pool */
    thrPool = (threadPool*) malloc(sizeof(threadPool));
    if ( thrPool == NULL ) {
        perror("Cannot allocate memory for thread pool");
        return NULL;
    }
    /* Set initial active and working thread number to 0 */
    thrPool->numActiveThreads = 0;
    thrPool->numWorkingThreads = 0;

    /* Initialize task queue */
    if ( initTaskQueue(&(thrPool->taskQ)) < 0 ) {
        free(thrPool);
        return NULL;
    }

    /* Allocate memory for threads in the pool */
    thrPool->threads = (thread**) malloc(numThreads * sizeof(thread*));
    if ( thrPool->threads == NULL ) {
        perror("Cannot allocate memory for threads in the pool");
        free(thrPool);
        return NULL;
    }

    /* Initialize mutex variable */
    pthread_mutex_init(&(thrPool->mutexThreadCnt), NULL);

    /* Thread initialization */
    for (i = 0; i < numThreads; i++) {
        threadInit(thrPool, &(thrPool->threads[i]), i);
    }

    //TODO: need mutex? maybe need condition of all threads active
    /* Wait for threads to be completely initialized */
    while(thrPool->numActiveThreads != numThreads)
        ;

#ifdef DEBUG
    printf("Number of active threads after initialization is %d.\n", thrPool->numActiveThreads);
#endif

    return thrPool;
}

int threadPoolEnqueue(threadPool* thrPool,void (*funcPtr)(void*), void* arg) {
    /* Create new task */
    task* newTask = (task*) malloc(sizeof(task));
    if ( newTask == NULL ) {
        perror("Cannot allocate memory for new task");
        return -1;
    }
    /* Add function and function arguments to task */
    newTask->function = funcPtr;
    newTask->arg = arg;

    /* Add task to task queue */
    enqueueTask(&(thrPool->taskQ), newTask);

    return 0;
}

void threadPoolWait(threadPool* thrPool) {
    pthread_mutex_lock(&(thrPool->mutexThreadCnt));
    while ( thrPool->taskQ.len || thrPool->numWorkingThreads ) {
        pthread_cond_wait(&(thrPool->allThreadsIdle), &(thrPool->mutexThreadCnt));
    }
    pthread_mutex_unlock(&(thrPool->mutexThreadCnt));
}

int threadInit(threadPool* thrPool, thread** threadPtr, int id) {
    *threadPtr = (thread*) malloc(sizeof(thread));
    if ( *threadPtr == NULL ) {
        perror("Cannot allocate memory for a thread in the pool");
        return -1;
    }

    (*threadPtr)->thrPool = thrPool;
    (*threadPtr)->id = id;

    /* Create and execute thread */
    pthread_create(&(*threadPtr)->pthread, NULL, (void*) threadFunc, (void*)(*threadPtr));
    return 0;
}

void* threadFunc(thread* threadPtr) {
    threadPool* thrPool = threadPtr->thrPool;

    /* Thread reaps itself after termination */
    pthread_detach(pthread_self());

    /* Increase number of active threads */
    pthread_mutex_lock(&(thrPool->mutexThreadCnt));
    thrPool->numActiveThreads++;
    pthread_mutex_unlock(&(thrPool->mutexThreadCnt));

    /* Start work */
    while(1) {
        /* Waiting for tasks */
        bSemWait(thrPool->taskQ.taskCheck);

        /* Increase number of working threads */
        pthread_mutex_lock(&(thrPool->mutexThreadCnt));
        thrPool->numWorkingThreads++;
        pthread_mutex_unlock(&(thrPool->mutexThreadCnt));

        /* Dequeue task from task queue */
        void (*funcExec)(void*);
        void* arg;
        task* taskPtr = dequeueTask(&(thrPool->taskQ));
        /* Execute the dequeued task */
        if ( taskPtr ) {
#ifdef DEBUG
            printf("Executing dequeued task...\n");
#endif
            funcExec = taskPtr->function;
            arg = taskPtr->arg;
            funcExec(arg);
            /* Free allocated memory for client socket descriptor */
            free(taskPtr->arg);
            /* Free allocated memory for task */
            free(taskPtr);
        }

        /* Decrease number of working threads */
        pthread_mutex_lock(&(thrPool->mutexThreadCnt));
        thrPool->numWorkingThreads--;
        /* Signal if all threads are idle now */
        if (!thrPool->numWorkingThreads) {
            pthread_cond_signal(&thrPool->allThreadsIdle);
        }
        pthread_mutex_unlock(&(thrPool->mutexThreadCnt));
    }
    /* Decrease number of active threads */
    pthread_mutex_lock(&(thrPool->mutexThreadCnt));
    thrPool->numActiveThreads--;
    pthread_mutex_unlock(&(thrPool->mutexThreadCnt));
    return NULL;
}
