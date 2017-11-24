#include "thr_pool.h"

/* Function prototypes */
int threadInit(threadPool* thrPool, thread** threadPtr, int id);
void* threadFunc(thread* threadPtr);

threadPool* threadPoolInit(int numThreads) {
    pthread_t tid;
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

    /* Wait for threads to be completely initialized */
    while(thrPool->numActiveThreads != numThreads)
        ;

    /* Create the signaling thread to notify working threads of available tasks */
    pthread_create(&tid, NULL, signalThreadFunc, (void*)&(thrPool->taskQ));

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

void* signalThreadFunc(void* arg) {
    taskQueue* taskQ = (taskQueue*) arg;
    /* Thread reaps itself after termination */
    pthread_detach(pthread_self());
    /* Start checking */
    while(1) {
        /* Lock mutex before checking */
        pthread_mutex_lock(&(taskQ->mutexQueueRW));
        /* If there is at least one task in queue, post it */
        if (taskQ->len) {
            clientInfo* clntInfo = (clientInfo*) taskQ->head->arg;
            bSemPost(taskQ->taskCheck, clntInfo->assignedThrId);
        }
        /* Release mutex after checking */
        pthread_mutex_unlock(&(taskQ->mutexQueueRW));
    }
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
        bSemWait(thrPool->taskQ.taskCheck, threadPtr->id);

        /* Increase number of working threads */
        pthread_mutex_lock(&(thrPool->mutexThreadCnt));
        thrPool->numWorkingThreads++;
        pthread_mutex_unlock(&(thrPool->mutexThreadCnt));

        /* Dequeue task from task queue */
        void (*funcExec)(void*);
        clientInfo* clntInfo;
        task* taskPtr = dequeueTask(&(thrPool->taskQ));
        /* Check if task is not NULL */
        if ( taskPtr ) {
#ifdef DEBUG
            printf("Executing dequeued task...\n");
#endif
            funcExec = taskPtr->function;
            clntInfo = (clientInfo*) taskPtr->arg;
            /* Write log about response information */
            writeLog(threadPtr->id, inet_ntoa(clntInfo->clientAddr.sin_addr), clntInfo->clientAddr.sin_port);
            /* Executing task */
            funcExec(&(clntInfo->clientSock));
            /* Free allocated memory for client info */
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

int writeLog(int threadId, char* ipAddr, int portNum) {
    char fileName[15];
    char threadIdStr[5];
    FILE* fp;
    time_t rawtime;
    struct tm* timeInfo;

    /* Get current time */
    time(&rawtime);
    timeInfo = localtime(&rawtime);

    /* Construct file name */
    sprintf(fileName, "thread_%d.log", threadId + 1);

    /* Open file */
    fp = fopen(fileName, "a");

    /* Write to file */
    fprintf(fp, "%02d:%02d:%02d %s:%d GET /index.html\n", timeInfo->tm_hour,
        timeInfo->tm_min, timeInfo->tm_sec, ipAddr, portNum);

    /* Close file after writing */
    fclose(fp);

    return 0;
}
