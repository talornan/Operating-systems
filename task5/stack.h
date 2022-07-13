#include <unistd.h>
#include <fcntl.h>

enum commands {
    UNKNOWN,
    PUSH,
    POP,
    TOP
};

typedef char element[1024];

typedef struct {
    size_t size;
    int shm_fd;
    struct flock lock;
} stack_st;

int stack_create(stack_st ** stack);

int stack_free(stack_st * stack);

int stack_push(stack_st * stack, const char * data);

int stack_pop(stack_st * stack);

int stack_top(stack_st * stack, char * output);

bool string_starts_with(const char * a, const char * b);

enum commands get_command_type(char * line);

void get_argument(char * line, char * argument);
