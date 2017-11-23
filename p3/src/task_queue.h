#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include "common.h"

#define THR_NUM 10

/*
 * Structure to store information of client,
 which is the arg element of the task
 */
typedef struct clientInfo {
    int clientSock;
    struct sockaddr_in clientAddr;
    unsigned int assignedThrId;
} clientInfo;

/* Binary semaphore */
typedef struct bSem {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int val[THR_NUM];    /* 0 means no tasks, 1 means tasks are available in queue */
} bSem;

/* Task */
typedef struct task {
    struct  task* prev;                 /* Pointer to previous task */
    void    (*function)(void* arg);     /* Function pointer to execute task */
    void*   arg;                        /* clientInfo passed to function pointer */
} task;

/* Task queue */
typedef struct taskQueue {
    pthread_mutex_t mutexQueueRW;       /* Mutex for task queue reading or writing */
    unsigned int len;                   /* Number of available tasks */
    task*   head;                       /* Head of task queue */
    task*   tail;                       /* Tail of task queue */
    bSem*   taskCheck;                  /* Check if task exists in queue */
} taskQueue;

/*
 * Function: initTaskQueue()
 * Initialize the task queue (FIFO)
 */
int initTaskQueue(taskQueue* taskQ);

/*
 * Function: enqueueTask()
 * Push a task pointed by newTask into the task queue
 */
void enqueueTask(taskQueue* taskQ, task* newTask);

/*
 * Function: dequeueTask()
 * Take a task out of the task queue
 * Return a task
 */
task* dequeueTask(taskQueue* taskQ);

/*
 * Function: bSemInit()
 * Initialize binary semaphore pointed
 by bSemPtr with value 'value'
 */
void bSemInit(bSem* bSemPtr, int value);

/*
 * Function: bSemPost()
 * Post the available task at the head of the queue
 to the thread having ID assignedThrId
 */
void bSemPost(bSem* bSemPtr, int assignedThrId);

/*
 * Function: bSemWait()
 * Wait for task to be posted
 * Check if the task is for the calling thread (by using thrId)
 * If it is, execute the task
 * If not, continue to wait on the condition variable
 */
void bSemWait(bSem* bSemPtr, int thrId);

#endif  /*__TASK_QUEUE_H__*/
