#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "active_object.h"

#define CONSUMER_THREADS_CNT 1

void* producer_thread(void* arument) {
    CQueueHead* queue_handle = (CQueueHead*)arument;
    for (int cnt_val = 0; cnt_val < CONSUMER_THREADS_CNT * 10; cnt_val++) {
        int* value = static_cast<int *>(malloc(sizeof(int)));
        memcpy(value, &cnt_val, sizeof(int));
        enQ(queue_handle, (void*)value);
        usleep(100);
    }
    printf("Thread producer stopped\n");
    pthread_exit(0);
}

void* processor(void* value) {
    printf("Processing value: %p\n", value);
    return value;
}

void* post_processor(void* value) {
    printf("Post-processing value: %p\n", value);
    return value;
}

int main() {
    pthread_t producer_thread_id = 0;
    CQueueHead* queue_handle = createQ();

    AO* ao = newAO(queue_handle, processor, post_processor);
    if (NULL == ao) {
        printf("Failed to create AO\n");
        exit(1);
    }

    if (0 != pthread_create(&producer_thread_id, NULL, producer_thread, queue_handle)) {
        printf("Failed to create thread\n");
        exit(1);
    }

    pthread_join(producer_thread_id, NULL);
    destroyAO(ao);
}
