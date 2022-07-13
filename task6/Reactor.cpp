#include "Reactor.h"

#define UNUSED(__x) ((void)(__x))


Reactor::Reactor() : _worker_thread([this] {run();}) {
}

Reactor::~Reactor() {
    _worker_thread.join();
}

void *Reactor::newReactor() {
    return new Reactor;
}

void Reactor::InstallHandler(int fd, Handler handler) {
    std::lock_guard<std::mutex> guard(_mutex);
    _handlers[fd] = handler;
}

void Reactor::RemoveHandler(int fd) {
    std::lock_guard<std::mutex> guard(_mutex);
    _handlers.erase(fd);
}

void Reactor::run() {
    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        struct timeval timeout {1, 0};

        int max_fd = 0;

        {
            std::lock_guard<std::mutex> guard(_mutex);
            for (auto const&[fd, _]: _handlers) {
                UNUSED(_);
                if (max_fd < fd) {
                    max_fd = fd;
                }
                FD_SET(fd, &read_fds);
            }
        }

        if (-1 == select(max_fd+1, &read_fds, nullptr, nullptr, &timeout))
        {
            throw std::runtime_error("Call to select failed");
        }

        std::unordered_map<int, Handler> handlers;
        {
            // Make a copy atomically
            std::lock_guard<std::mutex> guard(_mutex);
            handlers = _handlers;
        }
        for (auto const&[fd, handler]: handlers) {
            if (FD_ISSET(fd, &read_fds)) {
                handler(fd);
            }
        }
    }
}

std::vector<int> Reactor::get_fds() {
    std::lock_guard<std::mutex> guard(_mutex);
    std::vector<int> fds;

    for(const auto& [fd, _]: _handlers) {
        UNUSED(_);
        fds.push_back(fd);
    }
    return fds;
}
