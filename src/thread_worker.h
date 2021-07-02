#ifndef THREAD_WORKER_H_
#define THREAD_WORKER_H_

#include "tinycthread.h"

#include "map.h"
#include "chunk.h"
#include "linked_list.h"

typedef enum
{
    WORKER_IDLE,
    WORKER_BUSY,
    WORKER_DONE
}
WorkerState;

typedef struct
{
    Chunk* chunk;

    thrd_t thread;
    cnd_t cond_var;
    mtx_t state_mtx;
    WorkerState state;
    int mesh_rebuild;
}
WorkerData;

int worker_func(void* data);

#endif