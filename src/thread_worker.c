#include "thread_worker.h"
#include "map.h"

int worker_func(void* _data)
{
    WorkerData* data = (WorkerData*)_data;

    while (true)
    {
        mtx_lock(&data->state_mtx);
        while (data->state != WORKER_BUSY)
            cnd_wait(&data->cond_var, &data->state_mtx);
        mtx_unlock(&data->state_mtx);

        if (data->generate_terrain)
            chunk_generate_terrain(data->chunk);
        chunk_generate_mesh(data->chunk);

        mtx_lock(&data->state_mtx);
        data->state = WORKER_DONE;
        mtx_unlock(&data->state_mtx);
    }

    return 0;
}
