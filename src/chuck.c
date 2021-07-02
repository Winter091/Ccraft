#include "chunk.h"

#include "stdlib.h"

#include "block.h"
#include "utils.h"
#include "db.h"
#include "worldgen.h"

Chunk* chunk_init(int chunk_x, int chunk_z)
{
    Chunk* c = malloc(sizeof(Chunk));

    c->blocks = NULL;
    c->x = chunk_x;
    c->z = chunk_z;
    c->is_terrain_generated = 0;
    c->is_mesh_generated = 0;

    c->VAO_land = 0;
    c->VBO_land = 0;
    c->VAO_water = 0;
    c->VBO_water = 0;
    c->vertex_land_count = 0;
    c->vertex_water_count = 0;

    // Generate terrain
    //c->blocks = calloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH, 1);
    //worldgen_generate_chunk(c);

    //if (USE_MAP)
    //{
    //    // load block differences from database
    //    db_get_blocks_for_chunk(c);
    //}

    return c;
}

void chunk_generate_terrain(Chunk* c)
{
    c->blocks = calloc(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH, 1);
    worldgen_generate_chunk(c);
}

void chunk_rebuild_buffer(Chunk* c, Chunk* neighs[8])
{
    if (c->is_mesh_generated)
    {
        c->is_mesh_generated = 0;
        glDeleteVertexArrays(2, (const GLuint[]){c->VAO_land, c->VAO_water});
        glDeleteBuffers(2, (const GLuint[]){c->VBO_land, c->VBO_water});
    }

    // Keep static buffers in order not to
    // make a lot of big allocations
    static Vertex* vertices_land = NULL;
    static Vertex* vertices_water = NULL;
    if (!vertices_land && !vertices_water)
    {
        // space for all vertices in a chunk, a slight overkill
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
        unsigned char block = c->blocks[XYZ(x, y, z)];
        if (block == BLOCK_AIR)
            continue;

        int faces[6];
        int faces_visible = block_set_visible_faces(c, x, y, z, neighs, faces);

        // No faces are visible -> block is invisible!
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
            unsigned char block_above = BLOCK_AIR;
            if (y + 1 < CHUNK_HEIGHT)
                block_above = c->blocks[XYZ(x, y + 1, z)];

            // Water should be shorter only if there's air above
            int make_shorter = block_above == BLOCK_AIR ? 1 : 0;
            gen_cube_vertices(
                vertices_water, &curr_vertex_water_count, block_x, block_y, block_z,
                c->blocks[XYZ(x, y, z)], BLOCK_SIZE, make_shorter, faces, ao
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
                    c->blocks[XYZ(x, y, z)], BLOCK_SIZE, 0, faces, ao
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
    c->is_mesh_generated = 1;
}

int chunk_is_visible(int chunk_x, int chunk_z, vec4 planes[6])
{
    // Construct chunk aabb
    vec3 aabb[2];
    aabb[0][0] = chunk_x * CHUNK_SIZE;
    aabb[0][1] = 0.0f;
    aabb[0][2] = chunk_z * CHUNK_SIZE;
    aabb[1][0] = aabb[0][0] + CHUNK_SIZE;
    aabb[1][1] = CHUNK_HEIGHT * BLOCK_SIZE;
    aabb[1][2] = aabb[0][2] + CHUNK_SIZE;

    // And use this nice function to determine 
    // whether the aabb is visible to camera or not
    return glm_aabb_frustum(aabb, planes);
}

void chunk_delete(Chunk* c)
{
    if (c->is_mesh_generated)
    {
        glDeleteVertexArrays(2, (const GLuint[]){c->VAO_land, c->VAO_water});
        glDeleteBuffers(2, (const GLuint[]){c->VBO_land, c->VBO_water});
    }

    if (c->is_terrain_generated)
        free(c->blocks);

    free(c);
}
