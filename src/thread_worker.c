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

        chunk_generate_terrain(data->chunk);
        if (data->mesh_rebuild)
            chunk_rebuild_buffer(data->chunk);

        mtx_lock(&data->state_mtx);
        data->state = WORKER_DONE;
        mtx_unlock(&data->state_mtx);
    }

    return 0;
}
