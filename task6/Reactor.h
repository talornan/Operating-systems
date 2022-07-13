#ifndef REACTOR_H
#define REACTOR_H

#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>

typedef void(*Handler)(int);

class Reactor {
public:
    Reactor();
    ~Reactor();

    static void* newReactor();
    void InstallHandler(int fd, Handler handler);
    void RemoveHandler(int fd);

    std::vector<int> get_fds();

private:
    void run();

private:
    std::unordered_map<int, Handler> _handlers;
    std::thread _worker_thread;
    std::mutex _mutex;
};

#endif //REACTOR_H
