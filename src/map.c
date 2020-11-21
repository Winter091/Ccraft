#include "map.h"

#include "stdlib.h"

Map* map_create()
{
    Map* map = malloc(sizeof(Map));
    map->chunk_count = 0;
    map->chunks = realloc(NULL, 0);
    return map;
}

void map_add_chunk(Map* map, int chunk_x, int chunk_y)
{
    map->chunks = realloc(map->chunks, ++map->chunk_count * sizeof(Chunk*));
    map->chunks[map->chunk_count - 1] = chunk_create(chunk_x, chunk_y);
}

void map_delete_chunk(Map* map, int chunk_x, int chunk_y)
{

}
