#ifndef EX6_ACTIVE_OBJECT_H
#define EX6_ACTIVE_OBJECT_H

#include "queue.h"

typedef void* PVOID;
typedef PVOID(*action)(PVOID);

typedef struct AO {
    CQueueHead* queue_handle;
    action processor;
    action post_processor;
    pthread_t thread_id;
} AO;

AO* newAO(CQueueHead* queue_handle, action processor, action post_processor);
void destroyAO(AO* ao);

#endif //EX6_ACTIVE_OBJECT_H
