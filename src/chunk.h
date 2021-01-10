#ifndef CHUNK_H_
#define CHUNK_H_

#include "cglm/cglm.h"
#include "config.h"
#include "stdlib.h"
#include "glad/glad.h"

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
    GLuint VAO_land;
    GLuint VBO_land;
    GLuint VAO_water;
    GLuint VBO_water;
    size_t vertex_land_count;
    size_t vertex_water_count;
    int x, z;
    int is_loaded;
}
Chunk;

Chunk* chunk_init(int chunk_x, int chunk_z);
void chunk_generate(Chunk* c);
void chunk_update_buffer(Chunk* c, Chunk* neighs[8]);
int chunk_is_visible(int chunk_x, int chunk_z, vec4 planes[6]);
uint32_t chunk_hash_func(Chunk* c);
uint32_t chunk_hash_func2(int chunk_x, int chunk_z);
void chunk_delete(Chunk* c);

#endif