#include "glad/glad.h"
#include "map.h"

#include "stdlib.h"
#include "limits.h"
#include "shader.h"
#include "texture.h"
#include "perlin_noise.h"
#include "db.h"
#include "block.h"

LINKEDLIST_IMPLEMENTATION(Chunk*, chunks);
HASHMAP_IMPLEMENTATION(Chunk*, chunks, chunk_hash_func);

static Map* map;

static Chunk* map_get_chunk(int chunk_x, int chunk_z)
{
    uint32_t index = chunk_hash_func2(chunk_x, chunk_z) % map->chunks_active->array_size;
    LinkedListMap_chunks* bucket = map->chunks_active->array[index];
    
    LinkedListNodeMap_chunks* node = bucket->head;

    while (node && !(node->data->x == chunk_x && node->data->z == chunk_z))
        node = node->ptr_next;

    return node ? node->data : NULL;
}

static void map_update_chunk_buffer(Chunk* c, int update_neighbours)
{
    if (!c)
        return;
    
    Chunk* neighs[8] = 
    {
        map_get_chunk(c->x - 1, c->z    ),
        map_get_chunk(c->x    , c->z - 1),
        map_get_chunk(c->x + 1, c->z    ),
        map_get_chunk(c->x    , c->z + 1),
        map_get_chunk(c->x - 1, c->z - 1),
        map_get_chunk(c->x + 1, c->z - 1),
        map_get_chunk(c->x + 1, c->z + 1),
        map_get_chunk(c->x - 1, c->z + 1)
    };

    chunk_update_buffer(c, neighs);
    
    if (update_neighbours)
    {
        for (int i = 0; i < 4; i++)
            if (neighs[i])
                map_update_chunk_buffer(neighs[i], 0);
    }
}

static void map_load_chunk(int chunk_x, int chunk_z)
{
    Chunk* c = chunk_init(chunk_x, chunk_z);
    chunk_generate(c);
    hashmap_chunks_insert(map->chunks_active, c);
    map_update_chunk_buffer(c, 1);
}

static void map_delete_chunk(int chunk_x, int chunk_z)
{
    Chunk* c = map_get_chunk(chunk_x, chunk_z);
    if (c)
    {
        hashmap_chunks_remove(map->chunks_active, c);
        chunk_delete(c);
    }
}

static void map_unload_far_chunks(Camera* cam)
{
    int cam_chunk_x = cam->pos[0] / CHUNK_SIZE;
    int cam_chunk_z = cam->pos[2] / CHUNK_SIZE;
    
    LinkedList_chunks* chunks_to_delete = list_chunks_create();

    for (int i = 0; i < map->chunks_active->array_size; i++)
    {  
        LinkedListNodeMap_chunks* node = map->chunks_active->array[i]->head;
        for ( ; node; node = node->ptr_next)
        {
            Chunk* c = node->data;
            if (abs(c->x - cam_chunk_x) > CHUNK_UNLOAD_RADIUS || abs(c->z - cam_chunk_z) > CHUNK_UNLOAD_RADIUS)
            {
                list_chunks_push_front(chunks_to_delete, c);
            }
        }
    }

    LinkedListNode_chunks* node = chunks_to_delete->head;
    for ( ; node; node = node->ptr_next)
    {
        Chunk* c = node->data;
        map_delete_chunk(c->x, c->z);
    }

    list_chunks_delete(chunks_to_delete);
}

static void map_load_chunks(Camera* cam)
{
    int cam_chunk_x = cam->pos[0] / CHUNK_SIZE;
    int cam_chunk_z = cam->pos[2] / CHUNK_SIZE;
    
    // load one best chunk for now
    // best == closest visible chunk
    int best_score = INT_MIN;
    int best_chunk_x = 0, best_chunk_z = 0;

    for (int x = cam_chunk_x - CHUNK_LOAD_RADIUS; x <= cam_chunk_x + CHUNK_LOAD_RADIUS; x++)
    {
        for (int z = cam_chunk_z - CHUNK_LOAD_RADIUS; z <= cam_chunk_z + CHUNK_LOAD_RADIUS; z++)
        {
            Chunk* c = map_get_chunk(x, z);

            // add all visible loaded chunks to render list,
            // that list is being cleared vevry frame
            if (c)
            {
                if (chunk_is_visible(x, z, cam->frustum_planes))
                    list_chunks_push_front(map->chunks_to_render, c);
            }

            // find the best chunk that is not loaded yet
            else
            {
                int visible = chunk_is_visible(x, z, cam->frustum_planes);
                int dist = chunk_dist_to_player(x, z, cam_chunk_x, cam_chunk_z);

                int curr_score = visible ? (1 << 30) : 0;
                curr_score -= dist;

                if (curr_score >= best_score)
                {
                    best_chunk_x = x;
                    best_chunk_z = z;
                    best_score = curr_score;
                }
            }
        }   
    }

    if (best_score != INT_MIN)
        map_load_chunk(best_chunk_x, best_chunk_z);
}

void map_init()
{
    map = malloc(sizeof(Map));

    map->chunks_active     = hashmap_chunks_create(CHUNK_RENDER_RADIUS * CHUNK_RENDER_RADIUS * 1.2f);
    //map->chunks_to_load    = list_chunks_create();
    //map->chunks_to_unload  = list_chunks_create();
    //map->chunks_to_rebuild = list_chunks_create();
    map->chunks_to_render  = list_chunks_create();

    map->shader_chunks = create_shader_program(
        "shaders/chunk_vertex.glsl",
        "shaders/chunk_fragment.glsl"
    );

    map->texture_blocks = array_texture_create("textures/minecraft_blocks.png");
    if (!map->texture_blocks)
    {
        fprintf(stderr, "Texture was not loaded!\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // set world seed
    *perlin2d_get_world_seed() = rand() % 100000;
}

void map_render_chunks(Camera* cam)
{    
    glUseProgram(map->shader_chunks);
    shader_set_mat4(map->shader_chunks, "mvp_matrix", cam->vp_matrix);
    shader_set_int1(map->shader_chunks, "texture_sampler", 0);
    array_texture_bind(map->texture_blocks, 0);

    LinkedListNode_chunks* node = map->chunks_to_render->head;
    for ( ; node; node = node->ptr_next)
    {
        Chunk* c = node->data;
        glBindVertexArray(c->VAO);
        glDrawArrays(GL_TRIANGLES, 0, c->vertex_count);
    }
    list_chunks_clear(map->chunks_to_render);
}

// return chunk coordinate block exists in
static int chunked(int a)
{
    if (a >= 0) return a / CHUNK_WIDTH;
    return (a + 1) / CHUNK_WIDTH - 1;
}

// return block coordinate in a chunk ([0 - CHUNK_WIDTH))
static int blocked(int a)
{
    int block = a % CHUNK_WIDTH;
    if (block < 0) block += CHUNK_WIDTH;
    return block;
}

static unsigned char map_get_block(int x, int y, int z)
{
    Chunk* c = map_get_chunk(chunked(x), chunked(z));
    if (!c || !c->is_loaded) 
        return 0;

    return c->blocks[XYZ(blocked(x), y, blocked(z))];
}

static void map_set_block(int x, int y, int z, unsigned char block)
{
    int chunk_x = chunked(x);
    int chunk_z = chunked(z);
    
    Chunk* c = map_get_chunk(chunk_x, chunk_z);
    if (!c) return;

    int block_x = blocked(x);
    int block_z = blocked(z);

    c->blocks[XYZ(block_x, y, block_z)] = block;
    map_update_chunk_buffer(c, 0);
    
    // update neighbour if changed block was
    // on the edge of chunk
    if (block_x == 0)
    {
        Chunk* left = map_get_chunk(chunk_x - 1, chunk_z);
        map_update_chunk_buffer(left, 0);

        if (block_z == 0)
        {
            Chunk* backleft = map_get_chunk(chunk_x - 1, chunk_z - 1);
            map_update_chunk_buffer(backleft, 0);
        }

        else if (block_z == CHUNK_WIDTH - 1)
        {
            Chunk* frontleft = map_get_chunk(chunk_x - 1, chunk_z + 1);
            map_update_chunk_buffer(frontleft, 0);
        }
    }

    else if (block_x == CHUNK_WIDTH - 1)
    {
        Chunk* right = map_get_chunk(chunk_x + 1, chunk_z);
        map_update_chunk_buffer(right, 0);

        if (block_z == 0)
        {
            Chunk* backright = map_get_chunk(chunk_x + 1, chunk_z - 1);
            map_update_chunk_buffer(backright, 0);
        }

        else if (block_z == CHUNK_WIDTH - 1)
        {
            Chunk* frontright = map_get_chunk(chunk_x + 1, chunk_z + 1);
            map_update_chunk_buffer(frontright, 0);
        }
    }

    if (block_z == 0)
    {
        Chunk* back = map_get_chunk(chunk_x, chunk_z - 1);
        map_update_chunk_buffer(back, 0);
    }
    
    else if (block_z == CHUNK_WIDTH - 1)
    {
        Chunk* front = map_get_chunk(chunk_x, chunk_z + 1);
        map_update_chunk_buffer(front, 0);
    }
}

// squared distance between 2 points in space;
// sqrt() function is quite expensive
static int dist2(int a, int b, int c, int x, int y, int z)
{
    return (a - x) * (a - x) + (b - y) * (b - y) + (c - z) * (c - z);
}

void map_handle_left_mouse_click(Camera* cam)
{
    if (!cam->has_active_block)
        return;

    int x = cam->active_block[0];
    int y = cam->active_block[1];
    int z = cam->active_block[2];

    // if camera looks at block nearby, remove the block
    map_set_block(x, y, z, BLOCK_AIR);
    cam->has_active_block = 0;

#if USE_DATABASE
    // store block change in database
    db_insert_block(
        chunked(x), chunked(z),
        blocked(x), y, blocked(z),
        BLOCK_AIR
    );
#endif
}

void find_best_spot_to_place_block(
    Camera* cam, int x, int y, int z, 
    int* best_x, int* best_y, int* best_z, int* best_dist
)
{
    // position of camera in blocks
    int cam_x = cam->pos[0] / BLOCK_SIZE;
    int cam_y = cam->pos[1] / BLOCK_SIZE;
    int cam_z = cam->pos[2] / BLOCK_SIZE;
    
    if (camera_looks_at_block(cam, x, y, z) && map_get_block(x, y, z) == BLOCK_AIR)
    {
        int dist = dist2(cam_x, cam_y, cam_z, x, y, z);
        if (dist < *best_dist)
        {
            *best_dist = dist;
            *best_x = x;
            *best_y = y;
            *best_z = z;
        }
    }
}

void map_handle_right_mouse_click(Camera* cam)
{
    if (!cam->has_active_block)
        return;
    
    int x = cam->active_block[0];
    int y = cam->active_block[1];
    int z = cam->active_block[2];

    int best_x = 0, best_y = 0, best_z = 0;
    int best_dist = INT_MAX;

    // 6 potential spots around active block
    find_best_spot_to_place_block(cam, x - 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(cam, x + 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(cam, x, y - 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(cam, x, y + 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(cam, x, y, z - 1, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(cam, x, y, z + 1, &best_x, &best_y, &best_z, &best_dist);

    if (best_dist == INT_MAX)
        return;

    map_set_block(best_x, best_y, best_z, BLOCK_DIRT);

#if USE_DATABASE
    // store block change in database
    db_insert_block(
        chunked(best_x), chunked(best_z),
        blocked(best_x), best_y, blocked(best_z),
        BLOCK_DIRT
    );
#endif
}

static void map_set_camera_active_block(Camera* cam)
{
    // position of camera in blocks
    int cam_x = cam->pos[0] / BLOCK_SIZE;
    int cam_y = cam->pos[1] / BLOCK_SIZE;
    int cam_z = cam->pos[2] / BLOCK_SIZE;

    // best_* will store the block,
    // if it's found
    int best_x, best_y, best_z;
    int best_dist = INT_MAX;

    // iterate over each block around player
    for (int y = cam_y - BLOCK_BREAK_RADIUS; y <= cam_y + BLOCK_BREAK_RADIUS; y++)
    {
        if (y < 0 || y >= CHUNK_HEIGHT) continue;

        for (int x = cam_x - BLOCK_BREAK_RADIUS; x <= cam_x + BLOCK_BREAK_RADIUS; x++)
            for (int z = cam_z - BLOCK_BREAK_RADIUS; z <= cam_z + BLOCK_BREAK_RADIUS; z++)
            {
                if (camera_looks_at_block(cam, x, y, z))
                {
                    unsigned char block = map_get_block(x, y, z);
                    if (block == BLOCK_AIR)
                        continue;
                    
                    // we need closest block that camera is pointing to
                    int distance = dist2(cam_x, cam_y, cam_z, x, y, z);
                    if (distance < best_dist)
                    {
                        best_dist = distance;
                        best_x = x;
                        best_y = y;
                        best_z = z;
                    }
                }
            }
    }
    
    if (best_dist == INT_MAX)
    {
        cam->has_active_block = 0;
        return;
    }

    cam->has_active_block = 1;
    cam->active_block[0] = best_x;
    cam->active_block[1] = best_y;
    cam->active_block[2] = best_z;
}

void map_update(Camera* cam)
{
    //float curr_time = glfwGetTime();
    
    map_unload_far_chunks(cam);
    map_load_chunks(cam);
    map_set_camera_active_block(cam);

    //printf("%.6f\n", glfwGetTime() - curr_time);
}