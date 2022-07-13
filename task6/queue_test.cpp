// Taken from https://github.com/Thomseeen/tsqueue-c/blob/master/src/test.c

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

[[noreturn]] void* consumer_thread(void* arg);
void* producer_thread(void* arg);

#define CONSUMER_THREADS_CNT 3

CQueueHead* queue_handle;
pthread_t producer_thread_id;
pthread_t consumer_thread_id[CONSUMER_THREADS_CNT];

int main(int argc, char const* argv[]) {
    queue_handle = createQ();

    pthread_create(&producer_thread_id, NULL, producer_thread, NULL);

    for (int ii = 0; ii < CONSUMER_THREADS_CNT; ii++) {
        pthread_create(&consumer_thread_id[ii], NULL, consumer_thread,
                       (void*)&consumer_thread_id[ii]);
    }

    pthread_join(producer_thread_id, NULL);
    for (int ii = 0; ii < CONSUMER_THREADS_CNT; ii++) {
        pthread_join(consumer_thread_id[ii], NULL);
    }

    destroyQ(queue_handle);

    exit(0);
}

[[noreturn]] void* consumer_thread(void* arg) {
    int failed_cnt = 0;
    while (true) {
        void* value = deQ(queue_handle);
        // printf("Thread %lu reads queue len as %lu elements\n", *(pthread_t*)arg,
        //       cqueue_len(queue_handle));
        if (value) {
            printf("Thread %lu got item %d\n", *(pthread_t*)arg, *(int*)value);
        } else {
            failed_cnt++;
            if (failed_cnt > 100) {
                printf("Thread %lu stopped, no more values\n", *(pthread_t*)arg);
                break;
            }
        }
        usleep(100);
    }
    pthread_exit(0);
}

void* producer_thread(void* arg) {
    for (int cnt_val = 0; cnt_val < CONSUMER_THREADS_CNT * 5; cnt_val++) {
        int* value = static_cast<int *>(malloc(sizeof(int)));
        memcpy(value, &cnt_val, sizeof(int));
        enQ(queue_handle, (void*)value);
        usleep(100);
    }
    printf("Thread producer stopped, no more values\n");
    pthread_exit(0);
}
