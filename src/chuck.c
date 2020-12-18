#include "chunk.h"

#include "glad/glad.h"
#include "map.h"
#include "perlin_noise.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "GLFW/glfw3.h"
#include "db.h"
#include "block.h"

Chunk* chunk_init(int chunk_x, int chunk_z)
{
    Chunk* c = malloc(sizeof(Chunk));

    c->blocks = NULL;
    c->is_loaded = 0;
    c->VAO = 0;
    c->VBO = 0;
    c->vertex_count = 0;
    c->x = chunk_x;
    c->z = chunk_z;

    return c;
}

static unsigned int terrain_height_at(int x, int z)
{
    float freq = 0.0075f;

    float height = perlin2d(
        (float)x * freq, (float)z * freq, 
        8,             // octaves
        0.5f,          // persistence
        1.5f,          // lacunarity
        0.5f          // amplitude
    );

    height *= CHUNK_HEIGHT * 0.5098f;
    return height;
}

void chunk_generate(Chunk* c)
{
    const int grass_start = 50; 
    const int snow_start  = 80;
    
    c->blocks = malloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH);
    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        for (int z = 0; z < CHUNK_WIDTH; z++)
        {
            int block_x = x + c->x * CHUNK_WIDTH;
            int block_z = z + c->z * CHUNK_WIDTH;

            int air_start = terrain_height_at(block_x, block_z);
            for (int y = 0; y < CHUNK_HEIGHT; y++)
            {
                unsigned char block;

                if (y > air_start)
                    block = BLOCK_AIR;
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
                            block = BLOCK_GRASS;
                    }
                }

                c->blocks[XYZ(x, y, z)] = block;
            }         
        }
    }

    // load block differences from database
    db_update_chunk(c);
}

void chunk_update_buffer(
    Chunk* c, Chunk* left, Chunk* right, Chunk* front, Chunk* back
)
{
    if (c->is_loaded)
    {
        glDeleteVertexArrays(1, &c->VAO);
        glDeleteBuffers(1, &c->VBO);
    }
    
    Vertex* vertices = malloc(
        CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT * 36 * sizeof(Vertex)
    );

    size_t curr_vert_size = 0;
    int curr_vertex_count = 0;
    
    for (int x = 0; x < CHUNK_WIDTH; x++)
        for (int y = 0; y < CHUNK_HEIGHT; y++)
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                if (c->blocks[XYZ(x, y, z)] == BLOCK_AIR)
                    continue;

                int block_x = x + c->x * CHUNK_WIDTH;
                int block_y = y;
                int block_z = z + c->z * CHUNK_WIDTH;

                // a lot of faces are not visible from any
                // angle, so don't draw these faces
                int faces[6];
                block_set_visible_faces(
                    c, x, y, z, left, right, front, back, faces
                );

                int visible = 0;
                for (int i = 0; i < 6; i++)
                {
                    if (faces[i])
                    {
                        visible = 1;
                        break;
                    }
                }

                if (!visible)
                    continue;
                    
                block_gen_vertices(
                    vertices, curr_vertex_count, block_x, block_y, 
                    block_z, c->blocks[XYZ(x, y, z)], faces
                );

                for (int i = 0; i < 6; i++)
                {
                    if (faces[i])
                    {
                        curr_vertex_count += 6;
                        curr_vert_size += 6 * sizeof(Vertex);
                    }
                }
            }

    // Generate VAO and VBO, send vertices to videocard
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, curr_vert_size, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0
    );
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float))
    );
    glVertexAttribIPointer(
        2, 1, GL_UNSIGNED_BYTE,  sizeof(Vertex), (void*)(5 * sizeof(float))
    );
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    free(vertices);

    c->VAO = VAO;
    c->VBO = VBO;
    c->vertex_count = curr_vertex_count;
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

float chunk_dist_to_player(int chunk_x, int chunk_z, int pl_x, int pl_z)
{
    return abs(chunk_x - pl_x) + abs(chunk_z - pl_z);
}

uint32_t chunk_hash_func(Chunk* c)
{
    return (c->x + c->z) * (c->x + c->z + 1) / 2 + c->z;
}

uint32_t chunk_hash_func2(int chunk_x, int chunk_z)
{
    return (chunk_x + chunk_z) * (chunk_x + chunk_z + 1) / 2 + chunk_z;
}

void chunk_delete(Chunk* c)
{
    if (c->is_loaded)
    {
        free(c->blocks);
        glDeleteVertexArrays(1, &c->VAO);
        glDeleteBuffers(1, &c->VBO);
    }
    free(c);
}
