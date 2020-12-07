#ifndef CHUNK_H_
#define CHUNK_H_

#include "cglm/cglm.h"
#include "config.h"
#include "stdlib.h"

#define XYZ(x, y, z) ((x) * CHUNK_WIDTH * CHUNK_HEIGHT) + ((y) * CHUNK_WIDTH) + (z)

typedef struct
{
    unsigned char* blocks;
    unsigned int VAO;
    unsigned int VBO;
    size_t vertex_count;
    int x, z;
    int is_loaded;
}
Chunk;

Chunk* chunk_init(int chunk_x, int chunk_z);
void chunk_generate(Chunk* c);
void chunk_update_buffer(Chunk* c, Chunk* left, Chunk* right, Chunk* front, Chunk* back);
int chunk_is_visible(int chunk_x, int chunk_z, vec4 planes[6]);
float chunk_dist_to_player(int chunk_x, int chunk_z, int pl_x, int pl_z);
uint32_t chunk_hash_func(Chunk* c);
uint32_t chunk_hash_func2(int chunk_x, int chunk_z);
void chunk_delete(Chunk* c);

#endif