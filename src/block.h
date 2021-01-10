#ifndef BLOCK_H_
#define BLOCK_H_

#include "chunk.h"

#define BLOCKS_AMOUNT 24

// Block id's
#define BLOCK_AIR               0
#define BLOCK_STONE             1
#define BLOCK_DIRT              2
#define BLOCK_GRASS             3
#define BLOCK_WOODEN_PLANKS     4
#define BLOCK_POLISHED_STONE    5
#define BLOCK_BRICKS            6
#define BLOCK_COBBLESTONE       7
#define BLOCK_BEDROCK           8
#define BLOCK_SAND              9
#define BLOCK_GRAVEL            10
#define BLOCK_WOOD              11
#define BLOCK_IRON              12
#define BLOCK_GOLD              13
#define BLOCK_DIAMOND           14
#define BLOCK_EMERALD           15
#define BLOCK_REDSTONE          16
#define BLOCK_MOSSY_COBBLESTONE 17
#define BLOCK_OBSIDIAN          18
#define BLOCK_STONE_BRICKS      19
#define BLOCK_SNOW              20
#define BLOCK_SNOW_GRASS        21
#define BLOCK_GLASS             22
#define BLOCK_WATER             23
#define BLOCK_PLAYER_HAND       24

typedef struct
{
    float pos[3];
    float tex_coord[2];
    float ao;
    unsigned char tile;
}
Vertex;

extern unsigned char block_textures[][6];

int block_is_solid(unsigned char block);
int block_is_transparent(unsigned char block);
void block_get_neighs(Chunk* c, Chunk* neighs[8], int x, int y, int z, unsigned char b_neighs[27]);

void block_gen_vertices(
    Vertex* vertices, int curr_vertex_count, int x, int y, int z,
    int block_type, int center_align, float block_size, int faces[6], float ao[6][4]
);

int block_set_visible_faces(Chunk* c, int x, int y, int z, Chunk* neighs[8], int faces[6]);
void block_set_ao(unsigned char neighs[27], float ao[6][4]);

#endif