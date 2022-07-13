#include <stdio.h>
#include "Guard.h"

#define THREAD_COUNT 1000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* increment_counter_with_guard(void* argument) {
    Guard guard{&mutex};
    int * counter = (int*)argument;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        *counter = (*counter + 1);
    }
    return NULL;
}

void* increment_counter(void* argument) {
    int * counter = (int*)argument;
    for (int i = 0; i < THREAD_COUNT; ++i) {
        *counter = (*counter + 1);
    }
    return NULL;
}

void test_with_guard() {
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Failed pthread_mutex_init");
        return;
    }

    pthread_t threads[THREAD_COUNT] {};
    int counter = 0;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_create(&threads[i], NULL, increment_counter_with_guard, &counter);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    if (pthread_mutex_destroy(&mutex) != 0) {
        perror("Failed pthread_mutex_init");
        return;
    }

    printf("[test_with_guard] counter=%d\n", counter);
}

void test_without_guard() {
    pthread_t threads[THREAD_COUNT] {};
    int counter = 0;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_create(&threads[i], NULL, increment_counter, &counter);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("[test_without_guard] counter=%d\n", counter);
}

int main() {
    test_without_guard();
    test_with_guard();
    return 0;
}
