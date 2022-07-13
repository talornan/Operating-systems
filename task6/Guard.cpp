#include <stdexcept>
#include "Guard.h"

Guard::Guard(pthread_mutex_t* const lock_pointer) : _lock(lock_pointer) {
    lock();
}

Guard::~Guard() {
    try {
        unlock();
    } catch (...) {}
}

void Guard::lock() {
    if (pthread_mutex_lock(_lock) != 0) {
        throw std::runtime_error("Failed pthread_mutex_lock");
    }
}

void Guard::unlock() {
    if (pthread_mutex_unlock(_lock) != 0) {
        throw std::runtime_error("Failed pthread_mutex_unlock");
    }
}
