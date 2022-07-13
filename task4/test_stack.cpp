#include <stdio.h>
#include <string.h>
#include "stack.h"

#define TEST_DATA "DATA"

enum TestResult{
    UNKNOWN_FAILURE = -1,
    SUCCESS = 0,
    STACK_CREATE_FAILED,
    STACK_PUSH_FAILED,
    STACK_TOP_FAILED,
    STACK_TOP_UNEXPECTED_OUTPUT,
    STACK_POP_FAILED,
    STACK_TOP_EMPTY_FAILED,
    STACK_FREE_FAILED
};

int test(){
    int result = UNKNOWN_FAILURE;
    char output[1024] = {};
    const char * data = TEST_DATA;
    stack_st * stack = NULL;


    if (stack_create(&stack) != 0)
    {
        result = STACK_CREATE_FAILED;
        goto Exit;
    }

    if (stack_push(stack, data) != 0) {
        result = STACK_PUSH_FAILED;
        goto Exit;
    }

    if (stack_top(stack, output) != 0) {
        result = STACK_TOP_FAILED;
        goto Exit;
    }

    // Check that the output is the same as the pushed data
    if (strcmp(data, output) != 0) {
        result = STACK_TOP_UNEXPECTED_OUTPUT;
        goto Exit;
    }

    if (stack_pop(stack) != 0) {
        result = STACK_POP_FAILED;
        goto Exit;
    }

    // Check that the top fails when the stack is empty
    if (stack_top(stack, output) != -1) {
        result = STACK_TOP_EMPTY_FAILED;
        goto Exit;
    }

    result = SUCCESS;
Exit:
    if (stack_free(stack) != 0) {
        result = STACK_FREE_FAILED;
    }
    return result;
}

int main() {
    switch (test()) {
        case UNKNOWN_FAILURE:
            printf("UNKNOWN_FAILURE!\n");
            break;
        case SUCCESS:
            printf("SUCCESS!\n");
            break;
        case STACK_CREATE_FAILED:
            printf("STACK_CREATE_FAILED!\n");
            break;
        case STACK_PUSH_FAILED:
            printf("STACK_PUSH_FAILED!\n");
            break;
        case STACK_TOP_FAILED:
            printf("STACK_TOP_FAILED!\n");
            break;
        case STACK_TOP_EMPTY_FAILED:
            printf("STACK_TOP_EMPTY_FAILED!\n");
            break;
        case STACK_TOP_UNEXPECTED_OUTPUT:
            printf("STACK_TOP_UNEXPECTED_OUTPUT!\n");
            break;
        case STACK_POP_FAILED:
            printf("STACK_POP_FAILED!\n");
            break;
        case STACK_FREE_FAILED:
            printf("STACK_FREE_FAILED!\n");
            break;
        default:
            printf("Test returned unexpected value!\n");
    }
}
