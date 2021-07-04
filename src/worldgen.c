#include "worldgen.h"

#include "assert.h"
#include "stdlib.h"

#include "config.h"
#include "block.h"
#include "noise_generator.h"

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

static Biome get_biome(noise_state* state, int bx, int bz)
{
    // Voronoi noise
    state->fnl.cellular_return_type = FNL_CELLULAR_RETURN_VALUE_CELLVALUE;
    noise_set_settings(&state->fnl, FNL_NOISE_CELLULAR, 0.005f, 1, 2.0f, 0.5f);

    float fx = bx;
    float fz = bz;

    fnlDomainWarp2D(&state->fnl, &fx, &fz);
    float h = noise_2d(&state->fnl, fx, fz);

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

// Doesn't work on the edges of a chunk because
// some leaves would be in other chunks but we can
// only plae blocks in current chunk
static void make_tree(Chunk* c, int x, int y, int z)
{
    assert(x >= 2 && x <= CHUNK_WIDTH - 3 && z >= 2 && z <= CHUNK_WIDTH - 3);

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

                if (c->blocks[XYZ(bx, by, bz)] != BLOCK_AIR)
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

                if (c->blocks[XYZ(bx, by, bz)] != BLOCK_AIR)
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

                if (c->blocks[XYZ(bx, by, bz)] != BLOCK_AIR)
                    continue;
                
                c->blocks[XYZ(bx, by, bz)] = BLOCK_LEAVES;
            }
}

static void gen_plains(noise_state* state, Chunk* c, int x, int z, int h)
{
    for (int y = 0; y < h; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_DIRT;
    c->blocks[XYZ(x, h, z)] = BLOCK_GRASS;

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;

    // Generate grass and flowers only if there's no water
    if (c->blocks[XYZ(x, h + 1, z)] == BLOCK_WATER)
        return;

    if (my_rand(&state->rand_value) % 10 >= 7)
        c->blocks[XYZ(x, h + 1, z)] = BLOCK_GRASS_PLANT;
    else if (my_rand(&state->rand_value) % 100 > 97)
    {
        if (my_rand(&state->rand_value) % 2)
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_DANDELION;
        else
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_ROSE;
    }
}

static void gen_forest(noise_state* state, Chunk* c, int x, int z, int h)
{
    for (int y = 0; y < h; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_DIRT;
    c->blocks[XYZ(x, h, z)] = BLOCK_GRASS;

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
    
    if (c->blocks[XYZ(x, h + 1, z)] == BLOCK_WATER)
        return;

    if (my_rand(&state->rand_value) % 1000 > 975 
        && x >= 2 && z >= 2 && x <= CHUNK_WIDTH - 3 && z <= CHUNK_WIDTH - 3)
    {
        make_tree(c, x, h, z);
    }

    else if (my_rand(&state->rand_value) % 10 >= 9)
        c->blocks[XYZ(x, h + 1, z)] = BLOCK_GRASS_PLANT;
    
    else if (my_rand(&state->rand_value) % 100 > 97)
    {
        if (my_rand(&state->rand_value) % 2)
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_DANDELION;
        else
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_ROSE;
    }
}

static void gen_flower_forest(noise_state* state, Chunk* c, int x, int z, int h)
{
    for (int y = 0; y < h; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_DIRT;
    c->blocks[XYZ(x, h, z)] = BLOCK_GRASS;

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
    
    if (c->blocks[XYZ(x, h + 1, z)] == BLOCK_WATER)
        return;
    
    if (my_rand(&state->rand_value) % 1000 > 975 
        && x >= 2 && z >= 2 && x <= CHUNK_WIDTH - 3 && z <= CHUNK_WIDTH - 3)
    {
        make_tree(c, x, h, z);
    }

    else if (my_rand(&state->rand_value) % 10 >= 7)
    {
        int r = my_rand(&state->rand_value) % 3;
        if (r == 0)
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_GRASS_PLANT;
        else if (r == 1)
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_DANDELION;
        else
            c->blocks[XYZ(x, h + 1, z)] = BLOCK_FLOWER_ROSE;
    }

}

static void gen_mountains(noise_state* state, Chunk* c, int x, int z, int h)
{
    for (int y = 0; y <= h; y++)
    {
        if (y < 100 + my_rand(&state->rand_value) % 10 - 5)
        {
            if (my_rand(&state->rand_value) % 10 == 0)
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

static void gen_desert(noise_state* state, Chunk* c, int x, int z, int h)
{
    for (int y = 0; y < h; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_SANDSTONE;
    c->blocks[XYZ(x, h, z)] = BLOCK_SAND;

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
    
    if (c->blocks[XYZ(x, h + 1, z)] == BLOCK_WATER)
        return;

    if (my_rand(&state->rand_value) % 1000 > 995)
    {
        int cactus_height = my_rand(&state->rand_value) % 6;
        for (int y = 0; y < cactus_height; y++)
            c->blocks[XYZ(x, h + 1 + y, z)] = BLOCK_CACTUS;
    }
    else if (my_rand(&state->rand_value) % 1000 > 995)
        c->blocks[XYZ(x, h + 1, z)] = BLOCK_DEAD_PLANT;
}

static void gen_ocean(noise_state* state, Chunk* c, int x, int z, int h)
{
    for (int y = 0; y <= h; y++)
    {
        if (my_rand(&state->rand_value) % 4 == 0)
            c->blocks[XYZ(x, y, z)] = BLOCK_GRAVEL;
        else
            c->blocks[XYZ(x, y, z)] = BLOCK_SAND;
    }

    for (int y = h + 1; y <= water_level; y++)
        c->blocks[XYZ(x, y, z)] = BLOCK_WATER;
}

static int get_height(fnl_state* state, Biome biome, int bx, int bz)
{
    int v;
    
    switch (biome)
    {
        case BIOME_PLAINS:    
            noise_set_settings(state, FNL_NOISE_OPENSIMPLEX2, 0.003f, 3, 2.5f, 0.1f);
            v = noise_2d(state, bx, bz) * CHUNK_HEIGHT;
            return 44 + v / 8;    

        case BIOME_FOREST:   
            noise_set_settings(state, FNL_NOISE_OPENSIMPLEX2, 0.001f, 6, 4.0f, 0.75f);
            v = noise_2d(state, bx, bz) * CHUNK_HEIGHT * 0.7f;
            return 38 + v / 4;     

        case BIOME_FLOWER_FOREST: 
            noise_set_settings(state, FNL_NOISE_OPENSIMPLEX2, 0.001f, 6, 4.0f, 0.75f);
            v = noise_2d(state, bx, bz) * CHUNK_HEIGHT * 0.7f;
            return 38 + v / 4;     

        case BIOME_MOUNTAINS:  
            noise_set_settings(state, FNL_NOISE_OPENSIMPLEX2, 0.005f, 3, 2.0f, 1.0f);
            v = noise_2d(state, bx, bz) * CHUNK_HEIGHT;
            return 48 + v / 3;   

        case BIOME_DESERT:        
            noise_set_settings(state, FNL_NOISE_OPENSIMPLEX2, 0.0006f, 5, 2.5f, 0.75f);
            v = noise_2d(state, bx, bz) * CHUNK_HEIGHT;
            return 48 + v / 8;  

        case BIOME_OCEAN:         
            noise_set_settings(state, FNL_NOISE_OPENSIMPLEX2, 0.01f, 4, 2.0f, 0.5f);
            v = noise_2d(state, bx, bz) * CHUNK_HEIGHT;
            return 32 + v / 16;
        
        default:
            return CHUNK_HEIGHT - 1;
    }
}

// Bilinear interpolation
static int blerp(int h11, int h12, int h21, int h22, float x, float y)
{
    return h11 * (1 - x) * (1 - y) + h21 * x * (1 - y) + h12 * (1 - x) * y + h22 * x * y;
}

// Indexing into 'biomes' and 'heightmap' arrays
#define XZ(x, z) ((((x) + 8) * ((CHUNK_WIDTH + 1) + 8 + 8)) + ((z) + 8))

void worldgen_generate_chunk(Chunk* c)
{
    noise_state* state = noise_state_create(c->x, c->z);

    // Space for (CHUNK_WIDTH - 1) normal chunk blocks,
    // 2 blocks that are stored as neighbour data,
    // and 8 + 8 blocks for additional blocks used for
    // interpolation
    int const side_len = (CHUNK_WIDTH - 1) + 2 + 8 + 8;

    Biome* biomes = malloc(side_len * side_len * sizeof(Biome));
    int* heightmap = malloc(side_len * side_len * sizeof(int));

    int c_start_x = c->x * CHUNK_WIDTH;
    int c_start_z = c->z * CHUNK_WIDTH;

    for (int x = -8; x <= CHUNK_WIDTH + 8; x++)
    for (int z = -8; z <= CHUNK_WIDTH + 8; z++)
    {
        int bx = c_start_x + x;
        int bz = c_start_z + z;
        
        biomes[XZ(x, z)] = get_biome(state, bx, bz);

        if (x % 8 == 0 && z % 8 == 0)
            heightmap[XZ(x, z)] = get_height(&state->fnl, biomes[XZ(x, z)], bx, bz);
    }

    for (int x = -1; x <= CHUNK_WIDTH; x++)
    for (int z = -1; z <= CHUNK_WIDTH; z++)
    {
        if (x % 8 || z % 8)
        {
            int const x_left = floor8(x);
            int const z_top  = floor8(z);

            heightmap[XZ(x, z)] = blerp(
                heightmap[XZ(x_left, z_top)],
                heightmap[XZ(x_left, z_top + 8)],
                heightmap[XZ(x_left + 8, z_top)],
                heightmap[XZ(x_left + 8, z_top + 8)],
                (float)(x - x_left) / 8.0f, (float)(z - z_top) / 8.0f
            );
        }

        switch (biomes[XZ(x, z)])
        {
            case BIOME_PLAINS:        gen_plains(state, c, x, z,        heightmap[XZ(x, z)]); break;
            case BIOME_FOREST:        gen_forest(state, c, x, z,        heightmap[XZ(x, z)]); break;
            case BIOME_FLOWER_FOREST: gen_flower_forest(state, c, x, z, heightmap[XZ(x, z)]); break;
            case BIOME_MOUNTAINS:     gen_mountains(state, c, x, z,     heightmap[XZ(x, z)]); break;
            case BIOME_DESERT:        gen_desert(state, c, x, z,        heightmap[XZ(x, z)]); break;
            case BIOME_OCEAN:         gen_ocean(state, c, x, z,         heightmap[XZ(x, z)]); break;
        }
    }

    free(biomes);
    free(heightmap);

    free(state);
}
