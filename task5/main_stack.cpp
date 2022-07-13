#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"

int main(void) {

    int result = -1;
    char * line = NULL;
    size_t line_size = 0;
    char argument[1024] = {};
    char output[1024] = {};
    stack_st * stack = NULL;

    if (stack_create(&stack) == -1)
    {
        goto Exit;
    }

    do {
        if (getline(&line, &line_size, stdin) == -1)
        {
            goto Exit;
        }

        switch(get_command_type(line)) {
            case PUSH:
                get_argument(line, argument);
                stack_push(stack, argument);
                break;
            case POP:
                stack_pop(stack);
                break;
            case TOP:
                memset(output, '\0', sizeof(output));
                stack_top(stack, output);
                if (strlen(output)) {
                    printf("OUTPUT: %s\n", output);
                }
                break;
            default:
                printf("DEBUG: Unknown command.\n");
                break;
        }

    } while (true);

    result = 0;

    Exit:
    stack_free(stack);
    free(line);
    return result;
}
