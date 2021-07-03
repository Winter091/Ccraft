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
    int generate_terrain;

    WorkerState state;
    mtx_t state_mtx;
    cnd_t cond_var;
}
WorkerData;

typedef struct
{
    thrd_t thread;
    WorkerData data;
}
Worker;

int worker_func(void* data);

#endif