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
    /* Set initial active thread number to 0 */
    thrPool->numActiveThreads = 0;

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

    //TODO: need mutex?
    /* Wait for threads to be completely initialized */
    while(thrPool->numActiveThreads != numThreads)
        ;

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

int threadInit(threadPool* thrPool, thread** threadPtr, int id) {
    *threadPtr = (thread*) malloc(sizeof(thread));
    if ( *threadPtr == NULL ) {
        perror("Cannot allocate memory for a thread in the pool");
        return -1;
    }

    (*threadPtr)->thrPool = thrPool;
    (*threadPtr)->id = id;

    /* Create and execute thread */
    pthread_create(&(*threadPtr)->pthread, NULL, (void*) threadFunc, (*threadPtr));
    pthread_detach((*threadPtr)->pthread);
    return 0;
}

void* threadFunc(thread* threadPtr) {
    threadPool* thrPool = threadPtr->thrPool;
    /* Increase number of active threads */
    pthread_mutex_lock(&(thrPool->mutexThreadCnt));
    thrPool->numActiveThreads++;
    pthread_mutex_unlock(&(thrPool->mutexThreadCnt));

    /* Start work */
    while(1) {
        /* Waiting for tasks */
        bSemWait(thrPool->taskQ.taskCheck);
        /* Read task from task queue and execute it */
        void (*funcExec)(void*);
        void* arg;
        task* taskPtr = dequeueTask(&(thrPool->taskQ));
        if ( taskPtr ) {
            funcExec = taskPtr->function;
            arg = taskPtr->arg;
            funcExec(arg);
            free(taskPtr);
        }
    }
    /* Decrease number of active threads */
    pthread_mutex_lock(&(thrPool->mutexThreadCnt));
    thrPool->numActiveThreads--;
    pthread_mutex_unlock(&(thrPool->mutexThreadCnt));
    return NULL;
}
