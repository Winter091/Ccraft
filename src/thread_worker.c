#include "thread_worker.h"
#include "map.h"

int worker_loop(void* _data)
{
    WorkerData* data = (WorkerData*)_data;

    while (true)
    {
        mtx_lock(&data->state_mtx);
        while (data->state != WORKER_BUSY && data->state != WORKER_EXIT)
            cnd_wait(&data->cond_var, &data->state_mtx);

        if (data->state == WORKER_EXIT)
            break;
        
        mtx_unlock(&data->state_mtx);

        if (data->generate_terrain)
            chunk_generate_terrain(data->chunk);
        chunk_generate_mesh(data->chunk);

        mtx_lock(&data->state_mtx);
        if (data->state == WORKER_EXIT)
            break;
        data->state = WORKER_DONE;
        mtx_unlock(&data->state_mtx);
    }

    thrd_exit(0);
}

void worker_create(Worker* worker, thrd_start_t func)
{
    cnd_init(&worker->data.cond_var);
    mtx_init(&worker->data.state_mtx, mtx_plain);
    worker->data.state = WORKER_IDLE;
    worker->data.chunk = NULL;
    worker->data.generate_terrain = 0;
    
    thrd_create(&worker->thread, func, &worker->data);
}

void worker_destroy(Worker* worker)
{
    mtx_lock(&worker->data.state_mtx);
    worker->data.state = WORKER_EXIT;
    mtx_unlock(&worker->data.state_mtx);
    cnd_signal(&worker->data.cond_var);

    thrd_join(worker->thread, NULL);
    mtx_destroy(&worker->data.state_mtx);
    cnd_destroy(&worker->data.cond_var);
}