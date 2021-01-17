#ifndef BLOCK_H_
#define BLOCK_H_

#include "chunk.h"

// Block id's
#define BLOCK_AIR                0
#define BLOCK_PLAYER_HAND        1 
#define BLOCK_STONE              2 
#define BLOCK_DIRT               3 
#define BLOCK_GRASS              4 
#define BLOCK_WOODEN_PLANKS      5 
#define BLOCK_POLISHED_STONE     6 
#define BLOCK_BRICKS             7 
#define BLOCK_COBBLESTONE        8 
#define BLOCK_BEDROCK            9 
#define BLOCK_SAND               10
#define BLOCK_GRAVEL             11
#define BLOCK_WOOD               12
#define BLOCK_IRON               13
#define BLOCK_GOLD               14
#define BLOCK_DIAMOND            15
#define BLOCK_EMERALD            16
#define BLOCK_REDSTONE           17
#define BLOCK_MOSSY_COBBLESTONE  18
#define BLOCK_OBSIDIAN           19
#define BLOCK_STONE_BRICKS       20
#define BLOCK_SNOW               21
#define BLOCK_SNOW_GRASS         22
#define BLOCK_GLASS              23
#define BLOCK_WATER              24
#define BLOCK_LEAVES             25
#define BLOCK_GRASS_PLANT        26
#define BLOCK_FLOWER_ROSE        27
#define BLOCK_FLOWER_DANDELION   28
#define BLOCK_MUSHROOM_BROWN     29
#define BLOCK_MUSHROOM_RED       30
#define BLOCK_DEAD_PLANT         31
#define BLOCK_CACTUS             32
#define BLOCK_SANDSTONE          33
#define BLOCK_SANDSTONE_CHISELED 34
#define BLOCKS_AMOUNT            35

typedef struct
{
    float pos[3];
    float tex_coord[2];
    float ao;
    unsigned char tile;
}
Vertex;

extern unsigned char block_textures[][6];

void gen_cube_vertices(
    Vertex* vertices, int* curr_vertex_count, int x, int y, int z,
    int block_type, float block_size, int faces[6], float ao[6][4]
);

void gen_plant_vertices(
    Vertex* vertices, int* curr_vertex_count, int x, int y, int z,
    int block_type, float block_size
);

void block_gen_aabb(int x, int y, int z, vec3 aabb[2]);

int block_is_solid(unsigned char block);
int block_is_transparent(unsigned char block);
int block_is_plant(unsigned char block);
void block_get_neighs(Chunk* c, Chunk* neighs[8], int x, int y, int z, unsigned char b_neighs[27]);
int block_set_visible_faces(Chunk* c, int x, int y, int z, Chunk* neighs[8], int faces[6]);
void block_set_ao(unsigned char neighs[27], float ao[6][4]);

#endif