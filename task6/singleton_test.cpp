#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Singleton.h"

class MappedFile {
public:
    MappedFile() {
        _fd = open("README.md", O_RDONLY);
        if (_fd < 0) {
            throw std::runtime_error("Failed to open file");
        }

        if (fstat(_fd, &_stat) < 0) {
            throw std::runtime_error("Failed to fstat file");
        }

        _memory = mmap(nullptr, _stat.st_size, PROT_READ, MAP_PRIVATE, _fd, 0);
    }

    ~MappedFile() {
        try {
            munmap(_memory, _stat.st_size);
            close(_fd);
        } catch (...) {}
    }

    void* get_memory() {
        return _memory;
    }

private:
    int _fd;
    struct stat _stat;
    void* _memory;
};

Singleton<MappedFile> mapped_file;

int main() {
    printf("File content:\n%s\n", (char*)mapped_file.Instance()->get_memory());
}
