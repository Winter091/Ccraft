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
    WORKER_DONE,
    WORKER_EXIT
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

int worker_loop(void* data);

void worker_create(Worker* worker, thrd_start_t func);

void worker_destroy(Worker* worker);

#endif