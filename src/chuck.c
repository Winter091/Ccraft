#include "chunk.h"

#include "glad/glad.h"
#include "perlin_noise.h"
#include "stdlib.h"
#include "block.h"
#include "utils.h"
#include "db.h"
#include "string.h"

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

static unsigned int terrain_height_at(int x, int z)
{  
    float freq = 0.0075f;

    float height = perlin2d(
        (float)x * freq, (float)z * freq, 
        8,             // octaves
        0.5f,          // persistence
        1.5f,          // lacunarity
        0.5f           // amplitude
    );

    height *= CHUNK_HEIGHT * 0.5098f;
    return height;
}

Chunk* chunk_create(int chunk_x, int chunk_z)
{
    Chunk* c = malloc(sizeof(Chunk));

    c->blocks = NULL;
    c->x = chunk_x;
    c->z = chunk_z;
    c->is_loaded = 0;

    c->VAO_land = 0;
    c->VBO_land = 0;
    c->VAO_water = 0;
    c->VBO_water = 0;
    c->vertex_land_count = 0;
    c->vertex_water_count = 0;

    // Generate terrain
    const int water_end = 57;
    const int grass_start = 60; 
    const int snow_start  = 80;
    
    c->blocks = calloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH, 1);
    for (int x = 0; x < CHUNK_WIDTH; x++)
    for (int z = 0; z < CHUNK_WIDTH; z++)
    {
        int block_x = x + c->x * CHUNK_WIDTH;
        int block_z = z + c->z * CHUNK_WIDTH;

        unsigned int air_start = terrain_height_at(block_x, block_z);

        for (int y = 0; y < CHUNK_HEIGHT; y++)
        {
            if (c->blocks[XYZ(x, y, z)] != BLOCK_AIR)
                continue;
            
            unsigned char block;
            if (y > air_start)
            {
                if (y <= water_end)
                    block = BLOCK_WATER;
                else
                    block = BLOCK_AIR;
            }
            else
            {
                if (y < grass_start)
                    block = BLOCK_SAND;
                else
                    block = BLOCK_DIRT;
                if (block == BLOCK_DIRT && y == air_start)
                {
                    if (y >= snow_start)
                        block = BLOCK_SNOW_GRASS;
                    else
                    {
                        block = BLOCK_GRASS;
                        if (rand() % 10000 > 9950 && x >= 2 && x <= CHUNK_WIDTH - 3 && z >= 2 && z <= CHUNK_WIDTH - 3)
                            make_tree(c, x, y, z);
                        else if (rand() % 10 > 7)
                        {
                            if (rand() % 2)
                                c->blocks[XYZ(x, y + 1, z)] = BLOCK_FLOWER_DANDELION;
                            else
                                c->blocks[XYZ(x, y + 1, z)] = BLOCK_FLOWER_ROSE;
                        }
                    }
                    
                }
            }

            c->blocks[XYZ(x, y, z)] = block;
        }
    }

#if USE_DATABASE
    // load block differences from database
    db_get_blocks_for_chunk(c);
#endif

    return c;
}

void chunk_update_buffer(Chunk* c, Chunk* neighs[8])
{
    if (c->is_loaded)
    {
        glDeleteVertexArrays(2, (const GLuint[]){c->VAO_land, c->VAO_water});
        glDeleteBuffers(2, (const GLuint[]){c->VBO_land, c->VBO_water});
    }
    
    // Keep static buffers in order not to
    // make a lot of big allocations
    static Vertex* vertices_land = NULL;
    static Vertex* vertices_water = NULL;
    if (!vertices_land && !vertices_water)
    {
        // space for all vertices in a chunk
        vertices_land = malloc(
            CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT * 36 * sizeof(Vertex)
        );
        vertices_water = malloc(
            CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT * 36 * sizeof(Vertex)
        );
    }

    int curr_vertex_land_count = 0;
    int curr_vertex_water_count = 0;
    
    for (int x = 0; x < CHUNK_WIDTH; x++)
        for (int y = 0; y < CHUNK_HEIGHT; y++)
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                if (c->blocks[XYZ(x, y, z)] == BLOCK_AIR)
                    continue;

                int faces[6];
                int faces_visible = block_set_visible_faces(
                    c, x, y, z, neighs, faces
                );

                // block is not visible? Then don't draw it
                if (faces_visible == 0)
                    continue;

                unsigned char b_neighs[27];
                block_get_neighs(c, neighs, x, y, z, b_neighs);

                float ao[6][4];
                block_set_ao(b_neighs, ao);

                int block_x = x + c->x * CHUNK_WIDTH;
                int block_y = y;
                int block_z = z + c->z * CHUNK_WIDTH;

                if (c->blocks[XYZ(x, y, z)] == BLOCK_WATER)
                {
                    gen_cube_vertices(
                        vertices_water, &curr_vertex_water_count, block_x, block_y, block_z,
                        c->blocks[XYZ(x, y, z)], BLOCK_SIZE, faces, ao
                    );
                }
                else
                {
                    if (block_is_plant(c->blocks[XYZ(x, y, z)]))
                    {
                        gen_plant_vertices(
                            vertices_land, &curr_vertex_land_count, block_x, block_y, block_z,
                            c->blocks[XYZ(x, y, z)], BLOCK_SIZE
                        );
                    }
                    else
                    {
                        gen_cube_vertices(
                            vertices_land, &curr_vertex_land_count, block_x, block_y, block_z,
                            c->blocks[XYZ(x, y, z)], BLOCK_SIZE, faces, ao
                        );
                    }
                }
                
            }

    // Generate VAOs and VBOs
    c->VAO_land = opengl_create_vao();
    c->VBO_land = opengl_create_vbo(vertices_land, curr_vertex_land_count * sizeof(Vertex));;
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
    opengl_vbo_layout(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
    opengl_vbo_layout(2, 1, GL_UNSIGNED_BYTE, GL_FALSE,  sizeof(Vertex), 6 * sizeof(float));

    c->VAO_water = opengl_create_vao();
    c->VBO_water = opengl_create_vbo(vertices_water, curr_vertex_water_count * sizeof(Vertex));;
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
    opengl_vbo_layout(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
    opengl_vbo_layout(2, 1, GL_UNSIGNED_BYTE, GL_FALSE,  sizeof(Vertex), 6 * sizeof(float));

    c->vertex_land_count = curr_vertex_land_count;
    c->vertex_water_count = curr_vertex_water_count;
    c->is_loaded = 1;
}

int chunk_is_visible(int chunk_x, int chunk_z, vec4 planes[6])
{
    // construct chunk box
    vec3 aabb[2];
    aabb[0][0] = chunk_x * CHUNK_SIZE;
    aabb[0][1] = 0.0f;
    aabb[0][2] = chunk_z * CHUNK_SIZE;
    aabb[1][0] = aabb[0][0] + CHUNK_SIZE;
    aabb[1][1] = CHUNK_HEIGHT * BLOCK_SIZE;
    aabb[1][2] = aabb[0][2] + CHUNK_SIZE;

    // and use this nice function to determine 
    // whether the box is visible to camera or not
    return glm_aabb_frustum(aabb, planes);
}

void chunk_delete(Chunk* c)
{
    if (c->is_loaded)
    {
        free(c->blocks);
        glDeleteVertexArrays(2, (const GLuint[]){c->VAO_land, c->VAO_water});
        glDeleteBuffers(2, (const GLuint[]){c->VBO_land, c->VBO_water});
    }
    free(c);
}
