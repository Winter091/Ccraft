#ifndef CHUNK_H_
#define CHUNK_H_

#include "glad/glad.h"
#include "config.h"
#include "stdlib.h"

typedef struct
{
    unsigned char*** blocks;
    GLuint VAO;
    size_t vertex_count;
    int x, z;
}
Chunk;

Chunk* chunk_create(int chunk_x, int chunk_z);
void chunk_update_buffer(Chunk* c, Chunk* left, Chunk* right, Chunk* front, Chunk* back);

#endif