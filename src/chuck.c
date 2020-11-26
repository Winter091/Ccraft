#include "chunk.h"

#include "map.h"
#include "perlin_noise.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

typedef struct
{
    float pos[3];
    float tex_coord[2];
    unsigned char tile;
}
Vertex;

// ofsset = offset into vertices array
// x, y, z = cube world coordinates
// faces = determine whether to draw face or not
// tiles = choise of texture for each face
static void gen_cube_vertices(Vertex* vertices, int curr_vertex_count, int x, int y, int z, int block_type, int faces[6])
{    
    // row = face (6 faces), each face has 4 points forming a square
    static const float positions[6][4][3] = 
    {
        { {0, 0, 0 }, {0, 0, +1 }, {0, +1, 0 }, {0, +1, +1 } }, // left
        { {+1, 0, 0}, {+1, 0, +1}, {+1, +1, 0}, {+1, +1, +1} }, // right
        { {0, +1, 0}, {0, +1, +1}, {+1, +1, 0}, {+1, +1, +1} }, // top
        { {0, 0, 0 }, {0, 0, +1 }, {+1, 0, 0 }, {+1, 0, +1 } }, // bottom
        { {0, 0, 0 }, {0, +1, 0 }, {+1, 0, 0 }, {+1, +1, 0 } }, // back
        { {0, 0, +1}, {0, +1, +1}, {+1, 0, +1}, {+1, +1, +1} }  // front
    };

    static const int indices[6][6] = 
    {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };

    static const float uvs[6][4][2] = 
    {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
    };

    // texture number in texture atlas; order:
    // left right top bottom front back
    static const int tiles[3][6] = 
    {
        { 0,  0,  0,  0,  0,  0},  // 0 = air (not used)
        {16, 16, 32,  0, 16, 16},  // 1 = grass
        { 1,  1,  1,  1,  1,  1}   // 2 = sand
    };
    
    int curr_vertex = 0;

    // for each face
    for (int f = 0; f < 6; f++)
    {
        if (!faces[f]) continue;
        
        // for each vertex
        for (int v = 0; v < 6; v++)
        {
            int index = indices[f][v];

            vertices[curr_vertex_count + curr_vertex].pos[0] = (positions[f][index][0] + x) * BLOCK_SIZE;
            vertices[curr_vertex_count + curr_vertex].pos[1] = (positions[f][index][1] + y) * BLOCK_SIZE;
            vertices[curr_vertex_count + curr_vertex].pos[2] = (positions[f][index][2] + z) * BLOCK_SIZE;

            vertices[curr_vertex_count + curr_vertex].tex_coord[0] = uvs[f][index][0];
            vertices[curr_vertex_count + curr_vertex].tex_coord[1] = uvs[f][index][1];

            vertices[curr_vertex_count + curr_vertex].tile = tiles[block_type][f];

            curr_vertex++;
        }
    }
}

static inline unsigned char terrain_generation_func(int x, int y, int z)
{
    float value = perlin2d(x, z, 0.02, 8) * CHUNK_HEIGHT / 2.0f;

    if (y > value)
        return BLOCK_AIR;
    else if (y > 50)
        return BLOCK_GRASS;
    else
        return BLOCK_SAND;
}

static inline void block_set_visible_faces(Chunk* c, int x, int y, int z, Chunk* left, Chunk* right, Chunk* front, Chunk* back, int faces[6])
{
    // left
    if (x == 0)
    {
        if (!left)
            faces[0] = 1;
        else if (!left->blocks[CHUNK_WIDTH - 1][y][z])
            faces[0] = 1;
        else
            faces[0] = 0;
    }
    else
    {
        if (!c->blocks[x - 1][y][z])
            faces[0] = 1;
        else
            faces[0] = 0;
    }

    // right
    if (x == CHUNK_WIDTH - 1)
    {
        if (!right)
            faces[1] = 1;
        else if (!right->blocks[0][y][z])
            faces[1] = 1;
        else
            faces[1] = 0;
    }
    else
    {
        if (!c->blocks[x + 1][y][z])
            faces[1] = 1;
        else
            faces[1] = 0;
    }

    // top
    if (y == CHUNK_HEIGHT - 1)
    {
        faces[2] = 1;
    }
    else
    {
        if (!c->blocks[x][y + 1][z])
            faces[2] = 1;
        else
            faces[2] = 0;
    }

    // bottom
    if (y == 0)
    {
        faces[3] = 1;
    }
    else
    {
        if (!c->blocks[x][y - 1][z])
            faces[3] = 1;
        else
            faces[3] = 0;
    }

    // back
    if (z == 0)
    {
        if (!back)
            faces[4] = 1;
        else if (!back->blocks[x][y][CHUNK_WIDTH - 1])
            faces[4] = 1;
        else
            faces[4] = 0;
    }
    else
    {
        if (!c->blocks[x][y][z - 1])
            faces[4] = 1;
        else
            faces[4] = 0;
    }

    // front
    if (z == CHUNK_WIDTH - 1)
    {
        if (!front)
            faces[5] = 1;
        else if (!front->blocks[x][y][0])
            faces[5] = 1;
        else
            faces[5] = 0;
    }
    else
    {
        if (!c->blocks[x][y][z + 1])
            faces[5] = 1;
        else
            faces[5] = 0;
    }
}

Chunk* chunk_create(int chunk_x, int chunk_z)
{
    Chunk* c = malloc(sizeof(Chunk));
    c->x = chunk_x;
    c->z = chunk_z;
    c->VAO = 0;
    c->vertex_count = 0;

    c->blocks = malloc(CHUNK_WIDTH * sizeof(char**));
    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        c->blocks[x] = malloc(CHUNK_HEIGHT * sizeof(char*));
        for (int y = 0; y < CHUNK_HEIGHT; y++)
        {
            c->blocks[x][y] = malloc(CHUNK_WIDTH * sizeof(char));
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                int block_x = x + chunk_x * CHUNK_WIDTH;
                int block_y = y;
                int block_z = z + chunk_z * CHUNK_WIDTH;

                // blocks array determine the type of
                // block at particular coordinate
                // for example, 0 is air, 1 is grass
                c->blocks[x][y][z] = terrain_generation_func(
                    block_x, block_y, block_z
                );           
            }
        }
    }

    return c;
}

void chunk_update_buffer(Chunk* c, Chunk* left, Chunk* right, Chunk* front, Chunk* back)
{
    Vertex* vertices = malloc(
        CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT * 36 * sizeof(Vertex)
    );

    size_t curr_vert_size = 0;
    int curr_vertex_count = 0;
    
    for (int x = 0; x < CHUNK_WIDTH; x++)
        for (int y = 0; y < CHUNK_HEIGHT; y++)
            for (int z = 0; z < CHUNK_WIDTH; z++)
            {
                if (c->blocks[x][y][z] == BLOCK_AIR)
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
                    
                gen_cube_vertices(
                    vertices, curr_vertex_count, block_x, block_y, 
                    block_z, c->blocks[x][y][z], faces
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE,  sizeof(Vertex), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    free(vertices);

    c->VAO = VAO;
    c->vertex_count = curr_vertex_count;     
}
