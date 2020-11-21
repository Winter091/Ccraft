#ifndef CHUNK_H_
#define CHUNK_H_

#include "glad/glad.h"
#include "config.h"

typedef struct
{
    unsigned char*** blocks;
    GLuint VAO;
    size_t vertex_count;
}
Chunk;

Chunk* chunk_create(int chunk_x, int chunk_z);

#endif