// Taken from https://github.com/Thomseeen/tsqueue-c/blob/master/src/cqueue.c

#include <cstdio>
#include "queue.h"

CQueueHead* createQ() {
    CQueueHead* handle = static_cast<CQueueHead *>(malloc(sizeof(CQueueHead)));
    handle->head = NULL;
    handle->tail = NULL;
    handle->len = 0;

    pthread_mutex_t* lock = static_cast<pthread_mutex_t *>(malloc(sizeof(pthread_mutex_t)));
    pthread_mutex_init(lock, NULL);
    handle->lock = lock;
    pthread_cond_t* cond = static_cast<pthread_cond_t *>(malloc(sizeof(pthread_cond_t)));
    *cond = PTHREAD_COND_INITIALIZER;
    handle->cond = cond;

    return handle;
}

void destroyQ(CQueueHead* handle) {
    pthread_mutex_destroy(handle->lock);
    free(handle->lock);
    free(handle);
}

void enQ(CQueueHead* handle, void* value) {
    CQueueItem* new_item_p = static_cast<CQueueItem *>(malloc(sizeof(CQueueItem)));
    new_item_p->value = value;
    new_item_p->next = NULL;

    pthread_mutex_lock(handle->lock);
    if (!handle->head) {
        handle->head = new_item_p;
        handle->tail = new_item_p;
    } else {
        handle->tail->next = new_item_p;
        handle->tail = new_item_p;
    }
    handle->len++;
    pthread_cond_signal(handle->cond);
    pthread_mutex_unlock(handle->lock);
}

void* deQ(CQueueHead* handle) {
    pthread_mutex_lock(handle->lock);
    void* return_val;
    while (!handle->head) {
        pthread_cond_wait(handle->cond, handle->lock);
    }

    if (!handle->head) {
        return_val = NULL;
    } else {
        CQueueItem* item_copy_p = handle->head;
        handle->head = static_cast<CQueueItem *>(item_copy_p->next);

        void* return_value_p = item_copy_p->value;

        free(item_copy_p);
        handle->len--;

        return_val = return_value_p;
        pthread_cond_signal(handle->cond);
    }
    pthread_mutex_unlock(handle->lock);
    return return_val;
}
