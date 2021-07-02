#ifndef UTILS_H_
#define UTILS_H_

#include "stdlib.h"

#include "cglm/cglm.h"
#include "glad/glad.h"
#include "config.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Vertex layout for storing block data in GPU
typedef struct
{
    float pos[3];
    float tex_coord[2];
    float ao;
    unsigned char tile;
}
Vertex;

GLuint opengl_create_vao();
GLuint opengl_create_vbo(const void* vertices, size_t buf_size);
GLuint opengl_create_vbo_cube();
GLuint opengl_create_vbo_quad();
GLuint opengl_create_fbo();

static inline void my_glm_vec3_set(vec3 vec, float f0, float f1, float f2)
{
    vec[0] = f0;
    vec[1] = f1;
    vec[2] = f2;
}

static inline void my_glm_ivec3_set(ivec3 vec, int i0, int i1, int i2)
{
    vec[0] = i0;
    vec[1] = i1;
    vec[2] = i2;
}

static inline int chunked(int coord)
{
    if (coord >= 0) return coord / CHUNK_WIDTH;
    return (coord + 1) / CHUNK_WIDTH - 1;
}

static inline int to_chunk_block(int coord)
{
    int block = coord % CHUNK_WIDTH;
    if (block < 0) block += CHUNK_WIDTH;
    return block;
}

static inline float blocked(float coord)
{
    return coord / BLOCK_SIZE;
}

static inline int chunk_player_dist2(int cx, int cz, int px, int pz)
{
    return (cx - px) * (cx - px) + (cz - pz) * (cz - pz);
}

static inline float block_player_dist2(int bx, int by, int bz, float px, float py, float pz)
{
    return (bx - px) * (bx - px) + (by - py) * (by - py) + (bz - pz) * (bz - pz);
}

static inline void opengl_vbo_layout(
    GLuint index, GLint size, GLenum type, GLboolean normalized,
    GLsizei stride, size_t offset
)
{
    if (type >= GL_BYTE && type <= GL_UNSIGNED_INT)
        glVertexAttribIPointer(index, size, type, stride, (const void*)offset);
    else
        glVertexAttribPointer(index, size, type, normalized, stride, (const void*)offset);

    glEnableVertexAttribArray(index);
}

#endif