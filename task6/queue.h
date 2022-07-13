// Taken from https://github.com/Thomseeen/tsqueue-c/blob/master/src/cqueue.h

#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct CQueueItem {
    void* next;
    void* value;
} CQueueItem;

typedef struct CQueueHead {
    CQueueItem* head;
    CQueueItem* tail;
    uint64_t len;
    pthread_mutex_t* lock;
    pthread_cond_t* cond;

} CQueueHead;

CQueueHead* createQ();
void destroyQ(CQueueHead* handle);
void enQ(CQueueHead* handle, void* value);
void* deQ(CQueueHead* handle);

#endif /* QUEUE_H */