#include "chunk.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

// #### #### #### #---
// p[0] p[1] p[2] tile
typedef struct
{
    float pos[3];
    unsigned char tile;
}
Vertex;

// ofsset = curr_vertex_count into vertices array
// x, y, z = cube world coordinates
// faces = determine whether to draw face or not
// tiles = choise of texture for each face
static void gen_cube_vertices(Vertex* vertices, int curr_vertex_count, int x, int y, int z, int faces[6], int tiles[6])
{    
    // row = face (6 faces), each face has 4 points forming a square
    static const float positions[6][4][3] = 
    {
        { {0, 0, 0 }, {0, 0, +1 }, {0, +1, 0 }, {0, +1, +1 } }, // left
        { {+1, 0, 0}, {+1, 0, +1}, {+1, +1, 0}, {+1, +1, +1} }, // right
        { {0, +1, 0}, {0, +1, +1}, {+1, +1, 0}, {+1, +1, +1} }, // top
        { {0, 0, 0 }, {0, 0, +1 }, {+1, 0, 0 }, {+1, 0, +1 } }, // bottom
        { {0, 0, 0 }, {0, +1, 0 }, {+1, 0, 0 }, {+1, +1, 0 } }, // front
        { {0, 0, +1}, {0, +1, +1}, {+1, 0, +1}, {+1, +1, +1} }  // back
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
            vertices[curr_vertex_count + curr_vertex].pos[1] = (positions[f][index][1] + z) * BLOCK_SIZE;
            vertices[curr_vertex_count + curr_vertex].pos[2] = (positions[f][index][2] + y) * BLOCK_SIZE;

            vertices[curr_vertex_count + curr_vertex].tile = tiles[f];

            curr_vertex++;
        }
    }
}

Chunk* chunk_create(int chunk_x, int chunk_y)
{
    Chunk* c = malloc(sizeof(Chunk));

    // probably i shouldn't allocate memory for each block,
    // because there's no need to add air blocks to vertices
    Vertex* vertices = malloc(CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT * 36 * sizeof(Vertex));

    size_t curr_vert_size = 0;
    int curr_vertex_count = 0;

    c->blocks = malloc(CHUNK_WIDTH * sizeof(char**));
    for (int x = 0; x < CHUNK_WIDTH; x++)
    {
        c->blocks[x] = malloc(CHUNK_WIDTH * sizeof(char*));
        for (int y = 0; y < CHUNK_WIDTH; y++)
        {
            c->blocks[x][y] = malloc(CHUNK_HEIGHT * sizeof(char));
            for (int z = 0; z < CHUNK_HEIGHT; z++)
            {
                // blocks array determine a type of
                // block at particular coordinate
                // for example, 0 is air, 1 is grass
                c->blocks[x][y][z] = 1;

                if (c->blocks[x][y][z])
                {

                    int block_x = x + chunk_x * CHUNK_WIDTH;
                    int block_y = y + chunk_y * CHUNK_WIDTH;
                    int block_z = z;

                    int faces[6] = {1, 1, 1, 1, 1, 1};
                    int tiles[6] = {1, 1, 2, 2, 3, 3};
                    gen_cube_vertices(vertices, curr_vertex_count, block_x, block_y, block_z, faces, tiles);

                    for (int i = 0; i < 6; i++)
                        if (faces[i])
                        {
                            curr_vertex_count += 6;
                            curr_vert_size += 6 * sizeof(Vertex);
                        }  
                }
                
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16, (void*)0);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 16, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    free(vertices);

    c->VAO = VAO;
    c->vertex_count = curr_vertex_count;

    return c;
}