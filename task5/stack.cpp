#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"


static int stack_lock(stack_st * stack) {
    memset (&stack->lock, 0, sizeof(stack->lock));
    stack->lock.l_type = F_WRLCK;
    return fcntl(stack->shm_fd, F_SETLKW, &stack->lock);
}

static int stack_unlock(stack_st * stack) {
    stack->lock.l_type = F_UNLCK;
    return fcntl(stack->shm_fd, F_SETLKW, &stack->lock);
}

int stack_create(stack_st ** stack){
    int result = -1;

    stack_st * new_stack = (stack_st *)mmap(NULL, sizeof(stack_st), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1,0);
    if (new_stack == NULL) {
        goto Exit;
    }
    memset(new_stack, 0, sizeof(*new_stack));
    new_stack->shm_fd = shm_open("/STACK_SHM", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (-1 == (new_stack->shm_fd)) {
        munmap(new_stack, sizeof(stack_st));
        goto Exit;
    }

    *stack = new_stack;

    result = 0;

Exit:
    return result;
}

int stack_free(stack_st * stack) {
    int result = -1;
    if (NULL == stack) {
        goto Exit;
    }

    if (stack_lock(stack) == -1) {
        goto Exit;
    }
    shm_unlink("/SHACK_SHM");
    stack->shm_fd = 0;
    stack->size = 0;
    munmap(stack, sizeof(stack_st));

    result = 0;

Exit:
    return result;
}

int stack_push(stack_st * stack, const char * data){
    int result = -1;
    element * new_data = NULL;
    void * old_data = NULL;
    size_t new_size = 0;

    if ((NULL == stack) || (NULL == data)){
        printf("ERROR: Push invalid parameters");
        goto Exit;
    }

    if (stack_lock(stack) == -1) {
        goto Exit;
    }

    new_size = (stack->size + 1) * sizeof(element);
    new_data = (element *)mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, stack->shm_fd, 0);
    if (NULL == new_data) {
        printf("ERROR: Push failed to reallocate buffer");
        stack_unlock(stack);
        goto Exit;
    }

    if (0 == stack->size) {
        if (-1 == ftruncate(stack->shm_fd, new_size)) {
            printf("ERROR: Failed to truncate shared memory fd\n");
            goto Exit;
        }
    } else {
        old_data = malloc(stack->size * sizeof(element));
        if (NULL == old_data) {
            printf("ERROR: Failed to allocate memory\n");
            goto Exit;
        }
        memcpy(old_data, new_data, stack->size * sizeof(element));

        if (-1 == ftruncate(stack->shm_fd, new_size)) {
            printf("ERROR: Failed to truncate shared memory fd\n");
            goto Exit;
        }
        memcpy(new_data, old_data, stack->size * sizeof(element));
    }

    strcpy(new_data[stack->size], data);
    munmap(new_data, new_size);
    stack->size += 1;
    if (stack_unlock(stack) == -1) {
        goto Exit;
    }

    result = 0;
Exit:
    return result;
}

int stack_pop(stack_st * stack){
    int result = -1;

    if (NULL == stack){
        goto Exit;
    }

    if (stack_lock(stack) == -1) {
        goto Exit;
    }
    if (0 == stack->size){
        stack_unlock(stack);
        goto Exit;
    }

    stack->size -= 1;
    if (stack_unlock(stack) == -1) {
        goto Exit;
    }

    result = 0;
Exit:
    return result;
}

int stack_top(stack_st * stack, char * const output){
    int result = -1;
    element * data = NULL;

    if (NULL == stack) {
        goto Exit;
    }

    if (stack_lock(stack) == -1) {
        goto Exit;
    }

    if (0 == stack->size) {
        stack_unlock(stack);
        goto Exit;
    }
    data = (element *)mmap(NULL, (stack->size) * sizeof(element), PROT_READ | PROT_WRITE, MAP_SHARED, stack->shm_fd,0);
    strcpy(output, data[stack->size - 1]);
    if (stack_unlock(stack) == -1) {
        goto Exit;
    }

    result = 0;
Exit:
    return result;
}

bool string_starts_with(const char * const a, const char * const b)
{
    return (strncmp(a, b, strlen(b)) == 0);
}

enum commands get_command_type(char * line) {
    if (string_starts_with(line, "PUSH ")) {
        return PUSH;
    }
    if (string_starts_with(line, "POP")) {
        return POP;
    }
    if (string_starts_with(line, "TOP")) {
        return TOP;
    }

    return UNKNOWN;
}

void get_argument(char * line, char * argument) {
    strcpy(argument, strchr(line, ' ') + 1);
    argument[strlen(argument)-1] = '\0';
}
