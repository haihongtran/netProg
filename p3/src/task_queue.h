#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__

#include "common.h"

/* Binary semaphore */
typedef struct bSem {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int thrId;  /* Thread being assigned task */
    int val;    /* 0 means no tasks, 1 means tasks are available */
} bSem;

/* Task */
typedef struct task {
    struct  task* prev;                 /* Pointer to previous task */
    void    (*function)(void* arg);     /* Function pointer to execute task */
    void*   arg;                        /* Arguments passed to function pointer */
} task;

/* Task queue */
typedef struct taskQueue {
    pthread_mutex_t mutexQueueRW;       /* Mutex for task queue reading or writing */
    unsigned int len;                   /* Number of available tasks */
    task*   head;                       /* Head of task queue */
    task*   tail;                       /* Tail of task queue */
    bSem*   taskCheck;                  /* Check if task exists in queue */
} taskQueue;

/* Function prototypes */
int initTaskQueue(taskQueue* taskQ);
void enqueueTask(taskQueue* taskQ, task* newTask);
task* dequeueTask(taskQueue* taskQ);

void bSemInit(bSem* bSemPtr, int value);
void bSemWait(bSem* bSemPtr);
void bSemPost(bSem* bSemPtr);

#endif  /*__TASK_QUEUE_H__*/
