#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "active_object.h"

[[noreturn]] void* consumer_thread(void* argument){
    AO* ao = (AO*)argument;
    while (true) {
        void* value = deQ(ao->queue_handle);
        if (value) {
            ao->post_processor(ao->processor(value));
        }
        usleep(100);
    }
    pthread_exit(0);
}

AO* newAO(CQueueHead* queue_handle, action processor, action post_processor) {
    AO* ao = (AO*)calloc(1, sizeof(*ao));
    if (NULL == ao) {
        perror("Failed to allocate AO");
        return NULL;
    }
    ao->queue_handle = queue_handle;
    ao->processor = processor;
    ao->post_processor = post_processor;
    if (0 != pthread_create(&ao->thread_id, NULL, consumer_thread, (void*)ao)) {
        perror("Failed to create thread in AO");
        free(ao);
        return NULL;
    }
    return ao;
}

void destroyAO(AO* ao) {
    pthread_cancel(ao->thread_id);
    destroyQ(ao->queue_handle);
    free(ao);
}

