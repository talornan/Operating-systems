#ifndef SINGLETON_H
#define SINGLETON_H
#include <memory>
#include "Guard.h"

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

template <typename T>
class Singleton {
public:
    static T* Instance();
    static void Destroy();

private:
    inline static std::shared_ptr<T> _value;
    inline static Mutex _lock;
};

template<typename T>
T* Singleton<T>::Instance() {
    Guard guard{_lock.get_mutex()};
    if (_value == nullptr) {
        _value = std::make_shared<T>();
    }
    return _value.get();
}

template<typename T>
void Singleton<T>::Destroy() {
    _value.reset();
}

#endif //SINGLETON_H
