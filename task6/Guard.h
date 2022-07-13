#ifndef GUARD_H
#define GUARD_H

#include <pthread.h>

// Guard can not be used to protect strtok because it can be called with multiple parameters simultaneously (process different strings) and
// if there is a shared code which is used by the program (i.e. SO) the imported code can use strtok without locking the mutex.
struct Guard
{
public:
    Guard(pthread_mutex_t * lock_pointer);
    ~Guard();

private:
    void lock();
    void unlock();

private:
    pthread_mutex_t * const _lock;
};

#endif //GUARD_H
