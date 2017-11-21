#include "task_queue.h"

int initTaskQueue(taskQueue* taskQ) {
    /* Initialize binary semaphore */
    taskQ->taskCheck = (bSem*) malloc(sizeof(bSem));
    if ( taskQ == NULL ) {
        perror("Cannot allocate memory for bSem");
        return -1;
    }
    bSemInit(taskQ->taskCheck, 0);

    /* Initialize length, head and tail pointers */
    taskQ->len  = 0;
    taskQ->head = NULL;
    taskQ->tail = NULL;

    /* Initialize read/write mutex for the queue */
    pthread_mutex_init(&(taskQ->mutexQueueRW), NULL);

    return 0;
}

void enqueueTask(taskQueue* taskQ, task* newTask) {
    /* Lock mutex before enqueue */
    pthread_mutex_lock(&(taskQ->mutexQueueRW));
    newTask->prev = NULL;

    switch(taskQ->len) {
        case 0: /* No task in the queue */
            taskQ->head = newTask;
            taskQ->tail = newTask;
            break;
        default:    /* At least one task in the queue */
            taskQ->tail->prev = newTask;
            taskQ->tail = newTask;
            break;
    }
    /* Increase number of tasks in queue */
    taskQ->len++;
    /* Post the added task */
    bSemPost(taskQ->taskCheck);
    /* Release mutex after enqueue */
    pthread_mutex_unlock(&(taskQ->mutexQueueRW));
}

task* dequeueTask(taskQueue* taskQ) {
    /* Lock mutex before dequeue */
    pthread_mutex_lock(&(taskQ->mutexQueueRW));
    task* taskPtr = taskQ->head;
    switch(taskQ->len) {
        case 0:
            /* Do nothing */
            break;
        case 1:
            taskQ->head = NULL;
            taskQ->tail = NULL;
            taskQ->len  = 0;
            break;
        default:
            taskQ->head = taskPtr->prev;
            taskQ->len--;
            /* Post available tasks */
            bSemPost(taskQ->taskCheck);
            break;
    }
    /* Release mutex after dequeue */
    pthread_mutex_unlock(&(taskQ->mutexQueueRW));
    return taskPtr;
}

void bSemInit(bSem* bSemPtr, int value) {
    int i;
    if (value < 0 || value > 1) {
        printf("[WARNING] Invalid semaphore value. Initializing value as 0.\n");
        value = 0;
    }
    pthread_mutex_init(&(bSemPtr->mutex), NULL);
    pthread_cond_init(&(bSemPtr->cond), NULL);
    // for (i = 0; i < THREAD_NUM; i++) {
    //     pthread_cond_init(&(bSemPtr->cond[i]), NULL);
    // }
    // bSemPtr->assignThr = 0;
    bSemPtr->val = value;
}

void bSemWait(bSem* bSemPtr) {
    pthread_mutex_lock(&(bSemPtr->mutex));
    while ( bSemPtr->val != 1 ) {
        pthread_cond_wait(&(bSemPtr->cond), &(bSemPtr->mutex));
    }
    bSemPtr->val = 0;
    pthread_mutex_unlock(&(bSemPtr->mutex));
}

void bSemPost(bSem* bSemPtr) {
    pthread_mutex_lock(&(bSemPtr->mutex));
    bSemPtr->val = 1;
    pthread_cond_signal(&(bSemPtr->cond));
    pthread_mutex_unlock(&(bSemPtr->mutex));
}
