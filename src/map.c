#include "glad/glad.h"
#include "map.h"

#include "stdlib.h"
#include "limits.h"
#include "shader.h"
#include "texture.h"
#include "perlin_noise.h"
#include "db.h"
#include "block.h"
#include "utils.h"
#include "math.h"

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

    MAP_FOREACH_ACTIVE_CHUNK_BEGIN(c)
    {
        if (chunk_player_dist2(c->x, c->z, cam_chunk_x, cam_chunk_z) > CHUNK_UNLOAD_RADIUS * CHUNK_UNLOAD_RADIUS)
            list_chunks_push_front(chunks_to_delete, c);
    }
    MAP_FOREACH_ACTIVE_CHUNK_END()

    LIST_FOREACH_CHUNK_BEGIN(chunks_to_delete, c)
    {
        map_delete_chunk(c->x, c->z);
    }
    LIST_FOREACH_CHUNK_END()

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
            if (chunk_player_dist2(x, z, cam_chunk_x, cam_chunk_z) > CHUNK_LOAD_RADIUS * CHUNK_LOAD_RADIUS)
                continue;
            
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
                int dist = chunk_player_dist2(x, z, cam_chunk_x, cam_chunk_z);

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

    map->VAO_skybox = opengl_create_vao();
    map->VBO_skybox = opengl_create_vbo_cube();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(0);

    // set world seed
    *perlin2d_get_world_seed() = 123;
}

// [0.0 - 1.0)
static double map_get_time()
{
#if DISABLE_TIME_FLOW
    return 0.0;
#else
    return 0.5 + remainder(glfwGetTime(), DAY_LENGTH) / (double)DAY_LENGTH;
#endif
}

static double map_get_blocks_light()
{    
    double time = map_get_time();

    // 0.85 day
    if (time < DAY_TO_EVN_START)
        return 0.85;
    
    // 0.85 day - evening 0.7
    else if (time < EVN_TO_NIGHT_START)
        return 0.7 + 0.15 * (1 - glm_smoothstep(DAY_TO_EVN_START, EVN_TO_NIGHT_START, time));
    
    // 0.7 evening - night 0.3
    else if (time < NIGHT_START)
        return 0.3 + 0.4 * (1 - glm_smoothstep(EVN_TO_NIGHT_START, NIGHT_START, time));

    // night 0.3
    else if (time < NIGHT_TO_DAY_START)
        return 0.3;
    
    // 0.3 night - day 0.85
    else
        return 0.3 + 0.55 * glm_smoothstep(NIGHT_TO_DAY_START, 1.0f, time);
}

void map_render_sky(Camera* cam)
{    
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, cam->pos);

    mat4 mvp_matrix;
    glm_mat4_mul(cam->vp_matrix, model, mvp_matrix);
    
    glUseProgram(shader_skybox);
    shader_set_mat4(shader_skybox, "mvp_matrix", mvp_matrix);

    shader_set_int1(shader_skybox, "texture_day_sampler", 0);
    shader_set_int1(shader_skybox, "texture_evening_sampler", 1);
    shader_set_int1(shader_skybox, "texture_night_sampler", 2);
    
    shader_set_float1(shader_skybox, "time", map_get_time());
    shader_set_float1(shader_skybox, "day_to_evn_start", DAY_TO_EVN_START);
    shader_set_float1(shader_skybox, "evn_to_night_start", EVN_TO_NIGHT_START);
    shader_set_float1(shader_skybox, "night_start", NIGHT_START);
    shader_set_float1(shader_skybox, "night_to_day_start", NIGHT_TO_DAY_START);

    skybox_texture_bind(texture_skybox_day, 0);
    skybox_texture_bind(texture_skybox_evening, 1);
    skybox_texture_bind(texture_skybox_night, 2);

    double time = map_get_time();
    printf("%lf\n", time);

    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(map->VAO_skybox);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

void map_render_chunks(Camera* cam)
{    
    glUseProgram(shader_block);
    shader_set_mat4(shader_block, "mvp_matrix", cam->vp_matrix);
    shader_set_float1(shader_block, "block_light", map_get_blocks_light());
    shader_set_int1(shader_block, "texture_sampler", 0);
    array_texture_bind(texture_blocks, 0);

    LIST_FOREACH_CHUNK_BEGIN(map->chunks_to_render, c)
    {
        glBindVertexArray(c->VAO);
        glDrawArrays(GL_TRIANGLES, 0, c->vertex_count);
    }
    LIST_FOREACH_CHUNK_END()

    list_chunks_clear(map->chunks_to_render);
}

unsigned char map_get_block(int x, int y, int z)
{
    Chunk* c = map_get_chunk(chunked(x), chunked(z));
    if (!c || !c->is_loaded) 
        return 0;

    return c->blocks[XYZ(blocked(x), y, blocked(z))];
}

void map_set_block(int x, int y, int z, unsigned char block)
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

void map_update(Camera* cam)
{
    //float curr_time = glfwGetTime();
    
    map_unload_far_chunks(cam);
    map_load_chunks(cam);

    //printf("%.6f\n", glfwGetTime() - curr_time);
}