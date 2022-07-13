#ifndef MUTEX_H
#define MUTEX_H

#include <stdexcept>

class Mutex {
public:
    Mutex() : _lock(PTHREAD_MUTEX_INITIALIZER) {
        if (pthread_mutex_init(&_lock, NULL) != 0) {
            throw std::runtime_error("Failed pthread_mutex_init");
        }
    }

    ~Mutex() {
        try {
            if (pthread_mutex_destroy(&_lock) != 0) {
                throw std::runtime_error("Failed pthread_mutex_init");
            }
        } catch (...) {
            // Exceptions are not allowed in destructors
        }
    }

    pthread_mutex_t * get_mutex() {
        return &_lock;
    }

private:
    pthread_mutex_t _lock;
};

#endif //MUTEX_H
