#include <stdio.h>
#include <string.h>

#include "stack.h"
#include "malloc.h"


int stack_create(stack_st ** stack){
    int result = -1;
    stack_st * new_stack = (stack_st *)calloc(1, sizeof(stack_st));
    if (new_stack == NULL) {
        goto Exit;
    }

    if (0 != pthread_mutex_init(&(new_stack->mutex), NULL)) {
        free(new_stack);
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

    pthread_mutex_lock(&stack->mutex);
    pthread_mutex_destroy(&stack->mutex);

    free(stack->data);
    stack->data = NULL;
    stack->size = 0;
    free(stack);

    result = 0;

Exit:
    return result;
}

int stack_push(stack_st * stack, const char * data){
    int result = -1;

    if ((NULL == stack) || (NULL == data)){
        printf("ERROR: Push invalid parameters");
        goto Exit;
    }

    pthread_mutex_lock(&stack->mutex);

    if (stack->data == NULL) {
        stack->data = (element *)calloc(1, sizeof(*(stack->data)));
    } else {
        stack->data = (element *)realloc(stack->data, (stack->size + 1) * sizeof(*(stack->data)));
    }

    if (NULL == stack->data) {
        pthread_mutex_unlock(&stack->mutex);
        goto Exit;
    }
    strcpy(stack->data[stack->size], data);
    stack->size += 1;
    pthread_mutex_unlock(&stack->mutex);

    result = 0;
Exit:
    return result;
}

int stack_pop(stack_st * stack){
    int result = -1;

    if (NULL == stack){
        goto Exit;
    }

    pthread_mutex_lock(&stack->mutex);
    if (0 == stack->size){
        pthread_mutex_unlock(&stack->mutex);
        goto Exit;
    }

    stack->size -= 1;
    pthread_mutex_unlock(&stack->mutex);

    result = 0;
Exit:
    return result;
}

int stack_top(stack_st * stack, char * const output){
    int result = -1;

    if (NULL == stack) {
        goto Exit;
    }

    pthread_mutex_lock(&stack->mutex);
    if ((NULL == stack->data) || (0 == stack->size)) {
        pthread_mutex_unlock(&stack->mutex);
        goto Exit;
    }

    strcpy(output, stack->data[stack->size - 1]);
    pthread_mutex_unlock(&stack->mutex);

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

