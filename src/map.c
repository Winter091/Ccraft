#include "map.h"

#include "stdlib.h"

Map* map_create()
{
    Map* map = malloc(sizeof(Map));
    map->chunk_count = 0;
    map->chunks = realloc(NULL, 0);
    return map;
}

void map_add_chunk(Map* map, int chunk_x, int chunk_z)
{
    map->chunks = realloc(map->chunks, ++map->chunk_count * sizeof(Chunk*));
    map->chunks[map->chunk_count - 1] = chunk_create(chunk_x, chunk_z);
}

Chunk* map_get_chunk(Map* map, int chunk_x, int chunk_z)
{
    for (int i = 0; i < map->chunk_count; i++)
        if (map->chunks[i]->x == chunk_x && map->chunks[i]->z == chunk_z)
            return map->chunks[i];

    return NULL;
}

void map_gen_chunks_buffers(Map* map)
{
    for (int i = 0; i < map->chunk_count; i++)
    {
        Chunk* c = map->chunks[i];
        Chunk* left  = map_get_chunk(map, c->x - 1, c->z);
        Chunk* right = map_get_chunk(map, c->x + 1, c->z);
        Chunk* front = map_get_chunk(map, c->x, c->z + 1);
        Chunk* back  = map_get_chunk(map, c->x, c->z - 1);
        
        chunk_update_buffer(map->chunks[i], left, right, front, back);
    }
}

void map_delete_chunk(Map* map, int chunk_x, int chunk_z)
{

}