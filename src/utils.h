#ifndef UTILS_H_
#define UTILS_H_

#include "glad/glad.h"
#include "config.h"
#include "stdlib.h"

GLuint opengl_create_vao();
GLuint opengl_create_vbo(void* vertices, size_t buf_size);
GLuint opengl_create_vbo_skybox();

static inline int chunked(int coord)
{
    if (coord >= 0) return coord / CHUNK_WIDTH;
    return (coord + 1) / CHUNK_WIDTH - 1;
}

static inline int blocked(int coord)
{
    int block = coord % CHUNK_WIDTH;
    if (block < 0) block += CHUNK_WIDTH;
    return block;
}

static inline int chunk_player_dist2(int cx, int cz, int px, int pz)
{
    return (cx - px) * (cx - px) + (cz - pz) * (cz - pz);
}

static inline int block_player_dist2(int bx, int by, int bz, int px, int py, int pz)
{
    return (bx - px) * (bx - px) + (by - py) * (by - py) + (bz - pz) * (bz - pz);
}

#endif