#include "chunk.h"

#include "stdlib.h"

#include "block.h"
#include "utils.h"
#include "db.h"
#include "worldgen.h"

Chunk* chunk_init(int cx, int cz)
{
    Chunk* c = malloc(sizeof(Chunk));

    c->blocks = NULL;
    c->x = cx;
    c->z = cz;

    mtx_init(&c->mtx, mtx_timed);

    c->is_dirty = 0;
    c->is_generated = 0;

    c->VAO_land = 0;
    c->VBO_land = 0;
    c->VAO_water = 0;
    c->VBO_water = 0;
    c->vertex_land_count = 0;
    c->vertex_water_count = 0;

    c->generated_mesh_terrain = NULL;
    c->generated_mesh_water = NULL;

    return c;
}

void chunk_generate_terrain(Chunk* c)
{
    c->blocks = calloc(CHUNK_WIDTH_REAL * CHUNK_WIDTH_REAL * CHUNK_HEIGHT_REAL, 1);
    worldgen_generate_chunk(c);
    db_get_blocks_for_chunk(c);
}

/*
Neighs array layout (view towards -Y):

----> +X   Top layer   Middle layer   Bottom layer
|          18 21 24    9  12 15       0 3 6
v          19 22 25    10 13 16       1 4 7
+Z         20 23 26    11 14 17       2 5 8
*/
static void block_get_neighs(Chunk* c, int x, int y, int z, unsigned char b_neighs[27])
{
    int index = 0;

    for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++)
    for (int dz = -1; dz <= 1; dz++, index++)
    {
        int const bx = x + dx;
        int const by = y + dy;
        int const bz = z + dz;

        b_neighs[index] = c->blocks[XYZ(bx, by, bz)];
    }
}

static int should_be_visible(unsigned char block, Chunk* c, 
                             int neigh_x, int neigh_y, int neigh_z)
{
    unsigned char neigh = c->blocks[XYZ(neigh_x, neigh_y, neigh_z)];
    return block_is_transparent(neigh) && block != neigh;
}

static int block_set_visible_faces(Chunk* c, int x, int y, int z, int faces[6])
{
    unsigned char block = c->blocks[XYZ(x, y, z)];
    
    faces[BLOCK_FACE_LFT] = should_be_visible(block, c, x - 1, y, z);
    faces[BLOCK_FACE_RGT] = should_be_visible(block, c, x + 1, y, z);
    faces[BLOCK_FACE_TOP] = should_be_visible(block, c, x, y + 1, z);
    faces[BLOCK_FACE_BTM] = should_be_visible(block, c, x, y - 1, z);
    faces[BLOCK_FACE_BCK] = should_be_visible(block, c, x, y, z - 1);
    faces[BLOCK_FACE_FRT] = should_be_visible(block, c, x, y, z + 1);

    int num_visible = 0;
    for (int i = 0; i < 6; i++)
        num_visible += faces[i];
    return num_visible;
}

void block_set_ao(unsigned char neighs[27], float ao[6][4])
{       
    // Neighbours indices for each vertex for each face
    static const unsigned char lookup[6][4][3] = 
    {
        { { 0,  1,  9}, { 2,  1, 11}, {18,  9, 19}, {20, 19, 11} }, // left
        { { 6, 15,  7}, { 8, 17,  7}, {24, 25, 15}, {26, 17, 25} }, // right
        { {18, 19, 21}, {20, 19, 23}, {24, 21, 25}, {26, 23, 25} }, // top
        { { 0,  1,  3}, { 2,  1,  5}, { 6,  3,  7}, { 8,  5,  7} }, // bottom
        { { 0,  3,  9}, {18,  9, 21}, { 6,  3, 15}, {24, 15, 21} }, // back
        { { 2,  5, 11}, {20, 23, 11}, { 8,  5, 17}, {26, 17, 23} }, // front
    };

    static const float curve[4] = { 0.0f, 0.33f, 0.66f, 1.0f };

    for (int f = 0; f < 6; f++)
    for (int v = 0; v < 4; v++)
    {
        int corner = block_is_transparent(neighs[lookup[f][v][0]]) ? 0 : 1;
        int side1  = block_is_transparent(neighs[lookup[f][v][1]]) ? 0 : 1;
        int side2  = block_is_transparent(neighs[lookup[f][v][2]]) ? 0 : 1;

        ao[f][v] = side1 && side2 ? curve[3] : curve[corner + side1 + side2];
    }
}

void chunk_generate_mesh(Chunk* c)
{
    c->generated_mesh_terrain = malloc(
        CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT * 36 * sizeof(Vertex));
    c->generated_mesh_water = malloc(
        CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT * 36 * sizeof(Vertex));

    int curr_vertex_land_count = 0;
    int curr_vertex_water_count = 0;
    
    for (int x = 0; x < CHUNK_WIDTH; x++)
    for (int y = 0; y < CHUNK_HEIGHT; y++)
    for (int z = 0; z < CHUNK_WIDTH; z++)
    {
        unsigned char block = c->blocks[XYZ(x, y, z)];
        if (block == BLOCK_AIR)
            continue;

        int faces[6];
        int num_visible = block_set_visible_faces(c, x, y, z, faces);

        if (num_visible == 0)
            continue;

        unsigned char b_neighs[27];
        block_get_neighs(c, x, y, z, b_neighs);

        float ao[6][4];
        block_set_ao(b_neighs, ao);

        int bx = x + (c->x * CHUNK_WIDTH);
        int by = y;
        int bz = z + (c->z * CHUNK_WIDTH);

        if (block == BLOCK_WATER)
        {
            unsigned char block_above = c->blocks[XYZ(x, y + 1, z)];
            int make_shorter = (block_above == BLOCK_AIR);

            gen_cube_vertices(c->generated_mesh_water, &curr_vertex_water_count, bx, 
                              by, bz, block, BLOCK_SIZE, make_shorter, faces, ao);
        }
        else
        {
            if (block_is_plant(block))
            {
                gen_plant_vertices(c->generated_mesh_terrain, &curr_vertex_land_count, 
                                   bx, by, bz, block, BLOCK_SIZE);
            }
            else
            {
                gen_cube_vertices(c->generated_mesh_terrain, &curr_vertex_land_count, 
                                  bx, by, bz, block, BLOCK_SIZE, 0, faces, ao);
            }
        }

    }

    c->vertex_land_count = curr_vertex_land_count;
    c->vertex_water_count = curr_vertex_water_count;
}

void chunk_upload_mesh_to_gpu(Chunk* c)
{
    if (c->is_generated)
    {
        glDeleteVertexArrays(2, (const GLuint[]){c->VAO_land, c->VAO_water});
        glDeleteBuffers(2, (const GLuint[]){c->VBO_land, c->VBO_water});
    }

    c->VAO_land = opengl_create_vao();
    c->VBO_land = opengl_create_vbo(c->generated_mesh_terrain, c->vertex_land_count * sizeof(Vertex));
    free(c->generated_mesh_terrain);
    c->generated_mesh_terrain = NULL;
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
    opengl_vbo_layout(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
    opengl_vbo_layout(2, 1, GL_UNSIGNED_BYTE, GL_FALSE,  sizeof(Vertex), 6 * sizeof(float));

    c->VAO_water = opengl_create_vao();
    c->VBO_water = opengl_create_vbo(c->generated_mesh_water, c->vertex_water_count * sizeof(Vertex));
    free(c->generated_mesh_water);
    c->generated_mesh_water = NULL;
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 3 * sizeof(float));
    opengl_vbo_layout(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), 5 * sizeof(float));
    opengl_vbo_layout(2, 1, GL_UNSIGNED_BYTE, GL_FALSE,  sizeof(Vertex), 6 * sizeof(float));

    c->is_generated = 1;
}

int chunk_is_visible(int cx, int cz, vec4 planes[6])
{
    // Construct chunk aabb
    vec3 aabb[2];
    aabb[0][0] = cx * CHUNK_SIZE;
    aabb[0][1] = 0.0f;
    aabb[0][2] = cz * CHUNK_SIZE;
    aabb[1][0] = aabb[0][0] + CHUNK_SIZE;
    aabb[1][1] = CHUNK_HEIGHT * BLOCK_SIZE;
    aabb[1][2] = aabb[0][2] + CHUNK_SIZE;

    return glm_aabb_frustum(aabb, planes);
}

void chunk_delete(Chunk* c)
{
    if (c->is_generated)
    {
        glDeleteVertexArrays(2, (const GLuint[]){c->VAO_land, c->VAO_water});
        glDeleteBuffers(2, (const GLuint[]){c->VBO_land, c->VBO_water});
        free(c->blocks);
    }

    if (c->generated_mesh_terrain)
    {
        free(c->generated_mesh_terrain);
        free(c->generated_mesh_water);
    }

    free(c);
}
