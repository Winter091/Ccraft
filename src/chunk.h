#ifndef CHUNK_H_
#define CHUNK_H_

#include "stdlib.h"

#include "cglm/cglm.h"
#include "glad/glad.h"
#include "tinycthread.h"

#include "utils.h"
#include "config.h"

// Access block by 3 coords from 1-dimensional array
#define XYZ(x, y, z)   (((x) + 1) * CHUNK_WIDTH_REAL * CHUNK_HEIGHT_REAL) \
                     + (((y) + 1) * CHUNK_WIDTH_REAL)                     \
                     +  ((z) + 1)                                          

typedef struct
{
    unsigned char* blocks;
    int x, z;

    int is_dirty;
    int is_generated;
    int is_safe_to_modify;

    GLuint VAO_land;
    GLuint VBO_land;
    GLuint VAO_water;
    GLuint VBO_water;
    size_t vertex_land_count;
    size_t vertex_water_count;

    Vertex* generated_mesh_terrain;
    Vertex* generated_mesh_water;
}
Chunk;

Chunk* chunk_init(int cx, int cz);

void chunk_generate_terrain(Chunk* c);

// Create VAOs and VBOs, send them to GPU
void chunk_generate_mesh(Chunk* c);

void chunk_upload_mesh_to_gpu(Chunk* c);

// Used during frustum culling
int chunk_is_visible(int cx, int cz, vec4 planes[6]);

void chunk_delete(Chunk* c);

// Hash functions used in chunk hashmap
static inline uint32_t chunk_hash_func2(int cx, int cz)
{
    return (cx + cz) * (cx + cz + 1) / 2 + cz;
}

static inline uint32_t chunk_hash_func(Chunk* c)
{
    return chunk_hash_func2(c->x, c->z);
}

#endif