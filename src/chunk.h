#ifndef CHUNK_H_
#define CHUNK_H_

#include "stdlib.h"

#include "cglm/cglm.h"
#include "glad/glad.h"
#include "config.h"
#include "tinycthread.h"

// Access block by 3 coords from 1-dimensional array
#define XYZ(x, y, z) ((x) * CHUNK_WIDTH * CHUNK_HEIGHT) + ((y) * CHUNK_WIDTH) + (z)

// Chunk neighbours order
#define CHUNK_NEIGH_L  0
#define CHUNK_NEIGH_B  1
#define CHUNK_NEIGH_R  2
#define CHUNK_NEIGH_F  3
#define CHUNK_NEIGH_BL 4
#define CHUNK_NEIGH_BR 5
#define CHUNK_NEIGH_FR 6
#define CHUNK_NEIGH_FL 7

typedef struct
{
    unsigned char* blocks;
    int x, z;
    int is_terrain_generated;
    int is_mesh_generated;

    GLuint VAO_land;
    GLuint VBO_land;
    GLuint VAO_water;
    GLuint VBO_water;
    size_t vertex_land_count;
    size_t vertex_water_count;
}
Chunk;

Chunk* chunk_init(int chunk_x, int chunk_z);

void chunk_generate_terrain(Chunk* c);

// Create VAOs and VBOs, send them to GPU
void chunk_rebuild_buffer(Chunk* c, Chunk* neighs[8]);

// Used during frustum culling
int chunk_is_visible(int chunk_x, int chunk_z, vec4 planes[6]);

// Hash functions used in chunk hashmap
static inline uint32_t chunk_hash_func(Chunk* c)
{
    return (c->x + c->z) * (c->x + c->z + 1) / 2 + c->z;
}

static inline uint32_t chunk_hash_func2(int chunk_x, int chunk_z)
{
    return (chunk_x + chunk_z) * (chunk_x + chunk_z + 1) / 2 + chunk_z;
}

void chunk_delete(Chunk* c);

#endif