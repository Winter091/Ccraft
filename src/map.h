#ifndef MAP_H_
#define MAP_H_

#include "chunk.h"

typedef struct
{
    Chunk** chunks;
    size_t chunk_count;
}
Map;

Map* map_create();
void map_add_chunk(Map* map, int chunk_x, int chunk_y);
void map_delete_chunk(Map* map, int chunk_x, int chunk_y);

#endif