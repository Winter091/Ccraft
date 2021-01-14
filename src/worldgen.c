#include "worldgen.h"

#include "config.h"
#include "block.h"
#include "stdlib.h"
#include "fastnoiselite.h"

static const int water_level = 50;

typedef enum
{
    BIOME_PLAINS,
    BIOME_FOREST,
    BIOME_FLOWER_FOREST,
    BIOME_MOUNTAINS, 
    BIOME_DESERT,
    BIOME_OCEAN
}
Biome;

static Biome get_biome(int bx, int bz)
{
    // Voronoi noise
    noise_set_return_type(FNL_CELLULAR_RETURN_VALUE_CELLVALUE);
    noise_set_settings(FNL_NOISE_CELLULAR, 0.005f, 1, 2.0f, 0.5f);

    float fx = bx;
    float fz = bz;

    noise_apply_warp(&fx, &fz);
    float h = noise_2d(fx, fz);

    if (h < 0.2f)
        return BIOME_OCEAN;
    else if (h < 0.45f)
        return BIOME_PLAINS;
    else if (h < 0.65f)
        return BIOME_FOREST;
    else if (h < 0.75f)
        return BIOME_FLOWER_FOREST;
    else if (h < 0.85f)
        return BIOME_MOUNTAINS;
    else
        return BIOME_DESERT;
}

// I should rewrite it, it's not effective
static void make_tree(Chunk* c, int x, int y, int z)
{
    for (int i = 1; i <= 5; i++)
        c->blocks[XYZ(x, y + i, z)] = BLOCK_WOOD;
    c->blocks[XYZ(x, y + 7, z)] = BLOCK_LEAVES;

    for (int dx = -2; dx <= 2; dx++)
        for (int dz = -1; dz <= 1; dz++)
            for (int dy = 4; dy <= 5; dy++)
            {
                int bx = x + dx;
                int by = y + dy;
                int bz = z + dz;

                if (bx < 0 || bx >= CHUNK_WIDTH || by < 0 || by >= CHUNK_HEIGHT || bz < 0 || bz >= CHUNK_WIDTH)
                    continue;
                else if (c->blocks[XYZ(bx, by, bz)] != BLOCK_AIR)
                    continue;
                
                c->blocks[XYZ(bx, by, bz)] = BLOCK_LEAVES;
            }

    for (int dx = -1; dx <= 1; dx++)
        for (int dz = -2; dz <= 2; dz++)
            for (int dy = 4; dy <= 5; dy++)
            {
                int bx = x + dx;
                int by = y + dy;
                int bz = z + dz;

                if (bx < 0 || bx >= CHUNK_WIDTH || by < 0 || by >= CHUNK_HEIGHT || bz < 0 || bz >= CHUNK_WIDTH)
                    continue;
                else if (c->blocks[XYZ(bx, by, bz)] != BLOCK_AIR)
                    continue;
                
                c->blocks[XYZ(bx, by, bz)] = BLOCK_LEAVES;
            }

    for (int dx = -1; dx <= 1; dx++)
        for (int dz = -1; dz <= 1; dz++)
            for (int dy = 3; dy <= 6; dy++)
            {
                int bx = x + dx;
                int by = y + dy;
                int bz = z + dz;

                if (bx < 0 || bx >= CHUNK_WIDTH || by < 0 || by >= CHUNK_HEIGHT || bz < 0 || bz >= CHUNK_WIDTH)
                    continue;
                else if (c->blocks[XYZ(bx, by, bz)] != BLOCK_AIR)
                    continue;
                
                c->blocks[XYZ(bx, by, bz)] = BLOCK_LEAVES;
            }
}

static void gen_plains(Chunk* c, int x, int z, int h)
{
    for (int y = 0; y < h; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_DIRT;
    c->blocks[XYZ(x, h, z)] = BLOCK_GRASS;

    if (rand() % 10 >= 7)
        c->blocks[XYZ(x, h + 1, z)] = BLOCK_GRASS_PLANT;
    else if (rand() % 100 > 97)
    {
        if (rand() % 2)
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_DANDELION;
        else
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_ROSE;
    }

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
}

static void gen_forest(Chunk* c, int x, int z, int h)
{
    for (int y = 0; y < h; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_DIRT;
    c->blocks[XYZ(x, h, z)] = BLOCK_GRASS;

    if (rand() % 1000 > 975 && x >= 2 && z >= 2 && x <= CHUNK_WIDTH - 3 && z <= CHUNK_WIDTH - 3)
        make_tree(c, x, h, z);

    else if (rand() % 10 >= 9)
        c->blocks[XYZ(x, h + 1, z)] = BLOCK_GRASS_PLANT;
    
    else if (rand() % 100 > 97)
    {
        if (rand() % 2)
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_DANDELION;
        else
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_ROSE;
    }

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
}

static void gen_flower_forest(Chunk* c, int x, int z, int h)
{
    for (int y = 0; y < h; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_DIRT;
    c->blocks[XYZ(x, h, z)] = BLOCK_GRASS;

    if (rand() % 1000 > 975 && x >= 2 && z >= 2 && x <= CHUNK_WIDTH - 3 && z <= CHUNK_WIDTH - 3)
        make_tree(c, x, h, z);

    else if (rand() % 10 >= 7)
    {
        int r = rand() % 3;
        if (r == 0)
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_GRASS_PLANT;
        else if (r == 1)
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_DANDELION;
        else
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_ROSE;
    }

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
}

static void gen_mountains(Chunk* c, int x, int z, int h)
{
    for (int y = 0; y <= h; y++)
    {
        if (y < 100 + rand() % 10 - 5)
        {
            if (rand() % 10 == 0)
                c->blocks[XYZ(x, y, z)] = BLOCK_GRAVEL;
            else
                c->blocks[XYZ(x, y, z)] = BLOCK_STONE;
        }
        else
            c->blocks[XYZ(x, y, z)] = BLOCK_SNOW;
    }

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
}

static void gen_desert(Chunk* c, int x, int z, int h)
{
    for (int y = 0; y < h; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_SANDSTONE;
    c->blocks[XYZ(x, h, z)] = BLOCK_SAND;

    if (rand() % 1000 > 995)
    {
        int cactus_height = rand() % 6;
        for (int y = 0; y < cactus_height; y++)
            c->blocks[XYZ(x, h + 1 + y, z)] = BLOCK_CACTUS;
    }
    else if (rand() % 1000 > 995)
        c->blocks[XYZ(x, h + 1, z)] = BLOCK_DEAD_PLANT;

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
}

static void gen_ocean(Chunk* c, int x, int z, int h)
{
    for (int y = 0; y <= h; y++)
    {
        if (rand() % 4 == 0)
            c->blocks[XYZ(x, y, z)] = BLOCK_GRAVEL;
        else
            c->blocks[XYZ(x, y, z)] = BLOCK_SAND;
    }

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
}

static int get_height(Biome biome, int bx, int bz)
{
    int v;
    
    switch (biome)
    {
        case BIOME_PLAINS:    
            noise_set_settings(FNL_NOISE_OPENSIMPLEX2, 0.003f, 3, 2.5f, 0.1f);
            v = noise_2d(bx, bz) * CHUNK_HEIGHT;
            return 44 + v / 8;    

        case BIOME_FOREST:   
            noise_set_settings(FNL_NOISE_OPENSIMPLEX2, 0.001f, 6, 4.0f, 0.75f);
            v = noise_2d(bx, bz) * CHUNK_HEIGHT * 0.7f;
            return 38 + v / 4;     

        case BIOME_FLOWER_FOREST: 
            noise_set_settings(FNL_NOISE_OPENSIMPLEX2, 0.001f, 6, 4.0f, 0.75f);
            v = noise_2d(bx, bz) * CHUNK_HEIGHT * 0.7f;
            return 38 + v / 4;     

        case BIOME_MOUNTAINS:  
            noise_set_settings(FNL_NOISE_OPENSIMPLEX2, 0.005f, 3, 2.0f, 1.0f);
            v = noise_2d(bx, bz) * CHUNK_HEIGHT;
            return 48 + v / 3;   

        case BIOME_DESERT:        
            noise_set_settings(FNL_NOISE_OPENSIMPLEX2, 0.0006f, 5, 2.5f, 0.75f);
            v = noise_2d(bx, bz) * CHUNK_HEIGHT;
            return 48 + v / 8;  

        case BIOME_OCEAN:         
            noise_set_settings(FNL_NOISE_OPENSIMPLEX2, 0.01f, 4, 2.0f, 0.5f);
            v = noise_2d(bx, bz) * CHUNK_HEIGHT;
            return 32 + v / 16;
        
        default:
            return CHUNK_HEIGHT - 1;
    }
}

static float blerp(float h11, float h12, float h21, float h22, float x, float y)
{
    return h11 * (1 - x) * (1 - y) + h21 * x * (1 - y) + h12 * (1 - x) * y + h22 * x * y;
}

void worldgen_generate_chunk(Chunk* c)
{
    // Set seed for rand() in order to always get the
    // same grass/flowers/trees
    unsigned int seed = noise_get_seed();
    seed = ((seed >> 16) ^ c->x) * 0x45d9f3b;
    seed = ((seed >> 16) ^ c->z) * 0x45d9f3b;
    seed = (seed >> 16) ^ seed;
    srand(seed);
    
    /*
    Generate height only for some blocks (for every 8th block
    in a grid: (0, 0), (0, 8), (8, 0) etc), then interpolate
    these heights in order to get all remaining heights.
    */
    Biome biomes[CHUNK_WIDTH + 1][CHUNK_WIDTH + 1];
    int heightmap[CHUNK_WIDTH + 1][CHUNK_WIDTH + 1];
    
    int chunk_x_start = c->x * CHUNK_WIDTH;
    int chunk_z_start = c->z * CHUNK_WIDTH;

    for (int x = 0; x <= CHUNK_WIDTH; x++)
    for (int z = 0; z <= CHUNK_WIDTH; z++)
    {
        int bx = chunk_x_start + x;
        int bz = chunk_z_start + z;

        biomes[x][z] = get_biome(bx, bz);

        // Generate height using noise only for some blocks
        if (x % 8 == 0 && z % 8 == 0)
            heightmap[x][z] = get_height(biomes[x][z], bx, bz);
    }

    for (int x = 0; x < CHUNK_WIDTH; x++)
    for (int z = 0; z < CHUNK_WIDTH; z++)
    {
        // Interpolate generated heights over all blocks
        if (x % 8 || z % 8)
        {
            heightmap[x][z] = blerp(
                heightmap[x - (x % 8)][z - (z % 8)], 
                heightmap[x - (x % 8)][z - (z % 8) + 8],
                heightmap[x - (x % 8) + 8][z - (z % 8)], 
                heightmap[x - (x % 8) + 8][z - (z % 8) + 8], 
                (float)(x % 8) / 8.0f, (float)(z % 8) / 8.0f
            );
        }

        switch (biomes[x][z])
        {
            case BIOME_PLAINS:        gen_plains(c, x, z, heightmap[x][z]); break;
            case BIOME_FOREST:        gen_forest(c, x, z, heightmap[x][z]); break;
            case BIOME_FLOWER_FOREST: gen_flower_forest(c, x, z, heightmap[x][z]); break;
            case BIOME_MOUNTAINS:     gen_mountains(c, x, z, heightmap[x][z]); break;
            case BIOME_DESERT:        gen_desert(c, x, z, heightmap[x][z]); break;
            case BIOME_OCEAN:         gen_ocean(c, x, z, heightmap[x][z]); break; 

            //case BIOME_PLAINS:        c->blocks[XYZ(x, 2, z)] = BLOCK_GRASS; break;
            //case BIOME_FOREST:        c->blocks[XYZ(x, 2, z)] = BLOCK_WOOD; break;
            //case BIOME_FLOWER_FOREST: c->blocks[XYZ(x, 2, z)] = BLOCK_FLOWER_ROSE; break;
            //case BIOME_MOUNTAINS:     c->blocks[XYZ(x, 2, z)] = BLOCK_STONE; break;
            //case BIOME_DESERT:        c->blocks[XYZ(x, 2, z)] = BLOCK_SANDSTONE; break;
            //case BIOME_OCEAN:         c->blocks[XYZ(x, 2, z)] = BLOCK_WATER; break; 
        }
    }
}