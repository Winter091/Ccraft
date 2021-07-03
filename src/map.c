#include "map.h"

#include "stdlib.h"
#include "limits.h"
#include "math.h"

#include "shader.h"
#include "texture.h"
#include "fastnoiselite.h"
#include "utils.h"
#include "db.h"
#include "block.h"
#include "thread_worker.h"

// Define data structures for chunks
LINKEDLIST_DEFINITION(Chunk*, chunks);
HASHMAP_DEFINITION(Chunk*, chunks);

LINKEDLIST_IMPLEMENTATION(Chunk*, chunks);
HASHMAP_IMPLEMENTATION(Chunk*, chunks, chunk_hash_func);

// Useful defines that simplify iteration over chunks
#define MAP_FOREACH_ACTIVE_CHUNK_BEGIN(CHUNK_NAME)\
for (int i = 0; i < map->chunks_active->array_size; i++)\
{\
    LinkedListNodeMap_chunks* node = map->chunks_active->array[i]->head;\
    for ( ; node; node = node->ptr_next)\
    {\
        Chunk* CHUNK_NAME = node->data;\

#define MAP_FOREACH_ACTIVE_CHUNK_END() }}

#define LIST_FOREACH_CHUNK_BEGIN(LIST, CHUNK_NAME)\
{LinkedListNode_chunks* node = LIST->head;\
for ( ; node; node = node->ptr_next)\
{\
    Chunk* c = node->data;\

#define LIST_FOREACH_CHUNK_END() }}

typedef struct
{
    HashMap_chunks* chunks_active;
    LinkedList_chunks* chunks_to_render;

    GLuint VAO_skybox;
    GLuint VBO_skybox;

    GLuint VAO_sun_moon;
    GLuint VBO_sun_moon;

    Worker* workers;
    int num_workers;

    int seed;
}
Map;

// Keep static object for simplicity
static Map* map;

// NULL if chunk is not found
static Chunk* map_get_chunk(int chunk_x, int chunk_z)
{
    uint32_t index = chunk_hash_func2(chunk_x, chunk_z) % map->chunks_active->array_size;
    LinkedListMap_chunks* bucket = map->chunks_active->array[index];
    
    LinkedListNodeMap_chunks* node = bucket->head;

    while (node && !(node->data->x == chunk_x && node->data->z == chunk_z))
        node = node->ptr_next;

    return node ? node->data : NULL;
}

static void map_delete_chunk(int chunk_x, int chunk_z)
{
    Chunk* c = map_get_chunk(chunk_x, chunk_z);
    if (c)
    {
        hashmap_chunks_remove(map->chunks_active, c);
        chunk_delete(c);

        // Should rebuild neighbours but as long as all deleted
        // chunks are on edge of view, it's not nessessary
    }
}

static void try_delete_far_chunks(Camera* cam)
{
    int player_cx = chunked_cam(cam->pos[0]);
    int player_cz = chunked_cam(cam->pos[2]);
    
    LinkedList_chunks* chunks_to_delete = list_chunks_create();

    MAP_FOREACH_ACTIVE_CHUNK_BEGIN(c)
    {
        // If mtx is locked, the chunk is now being processed in
        // one of the worker threads, so can't safely delete it
        if (mtx_trylock(&c->mtx) != thrd_success)
            continue;
        mtx_unlock(&c->mtx);

        if (chunk_player_dist2(c->x, c->z, player_cx, player_cz) > CHUNK_UNLOAD_RADIUS2)
        {
            list_chunks_push_front(chunks_to_delete, c);
        }
    }
    MAP_FOREACH_ACTIVE_CHUNK_END()

    LIST_FOREACH_CHUNK_BEGIN(chunks_to_delete, c)
    {
        map_delete_chunk(c->x, c->z);
    }
    LIST_FOREACH_CHUNK_END()

    list_chunks_delete(chunks_to_delete);
}

void map_init()
{
    map = malloc(sizeof(Map));

    map->chunks_active    = hashmap_chunks_create(CHUNK_RENDER_RADIUS2 * 1.2f);
    map->chunks_to_render = list_chunks_create();

    map->VAO_skybox = opengl_create_vao();
    map->VBO_skybox = opengl_create_vbo_cube();
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    map->VAO_sun_moon = opengl_create_vao();
    map->VBO_sun_moon = opengl_create_vbo_quad();
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));

    map->seed = 0;
    db_get_map_info();

    map->num_workers = MAX(1, thread_hardware_concurrency() - 1);
    map->workers = malloc(map->num_workers * sizeof(Worker));
    
    for (int i = 0; i < map->num_workers; i++) 
        worker_create(&map->workers[i], worker_loop);
}

// [0.0 - 1.0)
double map_get_time()
{
    if (DISABLE_TIME_FLOW)
        return 0.1;
    else
        return 0.5 + remainder(glfwGetTime(), DAY_LENGTH) / (double)DAY_LENGTH;
}

static float map_get_blocks_light()
{    
    double time = map_get_time();

    if (time < EVN_TO_NIGHT_START)
        return glm_lerp(DAY_LIGHT, EVENING_LIGHT, glm_smoothstep(DAY_TO_EVN_START, EVN_TO_NIGHT_START, time));
    else if (time < NIGHT_TO_DAY_START)
        return glm_lerp(EVENING_LIGHT, NIGHT_LIGHT, glm_smoothstep(EVN_TO_NIGHT_START, NIGHT_START, time));
    else
        return glm_lerp(NIGHT_LIGHT, DAY_LIGHT, glm_smoothstep(NIGHT_TO_DAY_START, 1.0, time));
}

static void map_get_fog_color(float* r, float* g, float* b)
{    
    double time = map_get_time();
    static vec3 day_color     = {0.5f, 0.6f, 0.7f}; 
    static vec3 evening_color = {1.0f, 0.9f, 0.7f}; 
    static vec3 night_color   = {0.2f, 0.2f, 0.2f}; 
    vec3 color;

    if (time < EVN_TO_NIGHT_START)
        glm_vec3_mix(day_color, evening_color, glm_smoothstep(DAY_TO_EVN_START, EVN_TO_NIGHT_START, time), color);
    else if (time < NIGHT_TO_DAY_START)
        glm_vec3_mix(evening_color, night_color, glm_smoothstep(EVN_TO_NIGHT_START, NIGHT_START, time), color);
    else
        glm_vec3_mix(night_color, day_color, glm_smoothstep(NIGHT_TO_DAY_START, 1.0, time), color);

    *r = color[0];
    *g = color[1];
    *b = color[2];
}

void map_render_sun_moon(Camera* cam)
{
    mat4 model_sun, model_moon;

    glm_mat4_identity(model_sun);
    glm_translate(model_sun, cam->pos);
    glm_mat4_copy(model_sun, model_moon);

    double time = map_get_time();

    // apply offset to current time to synchronize
    // light level with sun presence in the sky
    double offset = 0.1;
    time += offset;
    if (time > 1.0)
        time -= 1.0;
    
    // sun and moon are always on the opposite
    // parts of the sky
    float angle_sun = time * GLM_PI * 2;
    float angle_moon = angle_sun + GLM_PI;

    // distance to the quads sun and moon are rendered on
    float dist = 5.0f;

    // move quads around player
    glm_translate(model_sun,  (vec3){0.0f, 0.0f, -dist * cosf(angle_sun)});
    glm_translate(model_sun,  (vec3){0.0f, dist * sinf(angle_sun), 0.0f});
    glm_translate(model_moon, (vec3){0.0f, 0.0f, -dist * cosf(angle_moon)});
    glm_translate(model_moon, (vec3){0.0f, dist * sinf(angle_moon), 0.0f});

    // rotate quads so sun and moon are always looking at player
    glm_rotate(model_sun,  angle_sun,  (vec3){1.0f, 0.0f, 0.0f});
    glm_rotate(model_moon, angle_moon, (vec3){1.0f, 0.0f, 0.0f});

    glUseProgram(shader_sun);
    glBindVertexArray(map->VAO_sun_moon);

    glDepthFunc(GL_ALWAYS);
    glEnable(GL_BLEND);

    mat4 mvp_matrix;
    glm_mat4_mul(cam->vp_matrix, model_sun, mvp_matrix);
    shader_set_mat4(shader_sun, "mvp_matrix", mvp_matrix);
    shader_set_texture_2d(shader_sun, "texture_sampler", texture_sun, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glm_mat4_mul(cam->vp_matrix, model_moon, mvp_matrix);
    shader_set_mat4(shader_sun, "mvp_matrix", mvp_matrix);
    shader_set_texture_2d(shader_sun, "texture_sampler", texture_moon, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDepthFunc(GL_LESS);
}

void map_render_sky(Camera* cam)
{    
    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, cam->pos);

    // rotate cubemap slightly, why not?
    glm_rotate(model, GLM_PI / 4.0f, (vec3){0.0f, 1.0f, 0.0f});

    mat4 mvp_matrix;
    glm_mat4_mul(cam->vp_matrix, model, mvp_matrix);
    
    glUseProgram(shader_skybox);
    shader_set_mat4(shader_skybox, "mvp_matrix", mvp_matrix);

    shader_set_texture_skybox(shader_skybox, "texture_day", texture_skybox_day, 0);
    shader_set_texture_skybox(shader_skybox, "texture_evening", texture_skybox_evening, 1);
    shader_set_texture_skybox(shader_skybox, "texture_night", texture_skybox_night, 2);
    
    shader_set_float1(shader_skybox, "time", map_get_time());
    shader_set_float1(shader_skybox, "day_to_evn_start", DAY_TO_EVN_START);
    shader_set_float1(shader_skybox, "evn_to_night_start", EVN_TO_NIGHT_START);
    shader_set_float1(shader_skybox, "night_start", NIGHT_START);
    shader_set_float1(shader_skybox, "night_to_day_start", NIGHT_TO_DAY_START);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glBindVertexArray(map->VAO_skybox);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

void map_render_chunks(Camera* cam)
{    
    glUseProgram(shader_block);

    shader_set_mat4(shader_block, "mvp_matrix", cam->vp_matrix);
    shader_set_float1(shader_block, "block_light", map_get_blocks_light());

    shader_set_texture_array(shader_block, "texture_sampler", texture_blocks, 0);

    shader_set_float3(shader_block, "cam_pos", cam->pos);
    shader_set_float1(shader_block, "fog_dist", CHUNK_RENDER_RADIUS * CHUNK_SIZE * 0.95f);

    float r, g, b;
    map_get_fog_color(&r, &g, &b);
    shader_set_float3(shader_block, "fog_color", (vec3){r, g, b});

    // Everything except water doesn't need blending
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    LIST_FOREACH_CHUNK_BEGIN(map->chunks_to_render, c)
    {
        glBindVertexArray(c->VAO_land);
        glDrawArrays(GL_TRIANGLES, 0, c->vertex_land_count);
    }
    LIST_FOREACH_CHUNK_END()

    // Water does need blending to be transparent, also
    // to see water from underneath we have to disable face culling
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    LIST_FOREACH_CHUNK_BEGIN(map->chunks_to_render, c)
    {
        glBindVertexArray(c->VAO_water);
        glDrawArrays(GL_TRIANGLES, 0, c->vertex_water_count);
    }
    LIST_FOREACH_CHUNK_END()
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);

    list_chunks_clear(map->chunks_to_render);
}

unsigned char map_get_block(int bx, int by, int bz)
{
    Chunk* c = map_get_chunk(chunked_block(bx), chunked_block(bz));
    if (!c || !c->is_generated) 
        return BLOCK_AIR;

    return c->blocks[XYZ(to_chunk_coord(bx), by, to_chunk_coord(bz))];
}

static void set_block_helper(int cx, int cz, int bx, int by, int bz, int block)
{
    db_insert_block(cx, cz, bx, by, bz, block);
    
    Chunk* c = map_get_chunk(cx, cz);
    if (c)
    {
        c->blocks[XYZ(bx, by, bz)] = block;
        c->is_dirty = 1;
    }
}

static void set_block(Chunk* c, int bx, int by, int bz, int block)
{
    set_block_helper(c->x, c->z, bx, by, bz, block);

    // Set in neighbour chunks if needed
    int const first = -1;
    int const last = CHUNK_WIDTH;

    if (bx == 0)
    {
        set_block_helper(c->x - 1, c->z, last, by, bz, block);
        
        if (bz == 0)
        {
            set_block_helper(c->x - 1, c->z - 1, last,  by, last, block);
            set_block_helper(c->x    , c->z - 1, 0, by, last, block);
        }
        else if (bz == CHUNK_WIDTH - 1)
        {
            set_block_helper(c->x - 1, c->z + 1, last,  by, first, block);
            set_block_helper(c->x    , c->z + 1, 0, by, first, block);
        }
    }
    else if (bx == CHUNK_WIDTH - 1)
    {
        set_block_helper(c->x + 1, c->z, first, by, bz, block);
        
        if (bz == 0)
        {
            set_block_helper(c->x + 1, c->z - 1, first, by, last, block);
            set_block_helper(c->x    , c->z - 1, last - 1,  by, last, block);
        }
        else if (bz == CHUNK_WIDTH - 1)
        {
            set_block_helper(c->x + 1, c->z + 1, first, by, first, block);
            set_block_helper(c->x    , c->z + 1, last - 1,  by, first, block);
        }
    }
    else
    {
        if (bz == 0)
            set_block_helper(c->x, c->z - 1, bx, by, last, block);
        else if (bz == CHUNK_WIDTH - 1)
            set_block_helper(c->x, c->z + 1, bx, by, first, block);
    }
}

void map_set_block(int bx, int by, int bz, unsigned char block)
{
    int cx = chunked_block(bx);
    int cz = chunked_block(bz);
    
    Chunk* c = map_get_chunk(cx, cz);
    if (!c) return;

    int x = to_chunk_coord(bx);
    int z = to_chunk_coord(bz);

    set_block(c, x, by, z, block);
}

static bool find_chunk_for_worker(Camera* cam, int* best_x, int* best_z)
{
    int player_cx = chunked_cam(cam->pos[0]);
    int player_cz = chunked_cam(cam->pos[2]);
    
    int best_score = INT_MAX;
    int best_cx = 0, best_cz = 0;
    int found = 0;

    for (int x = player_cx - CHUNK_LOAD_RADIUS; x <= player_cx + CHUNK_LOAD_RADIUS; x++)
    for (int z = player_cz - CHUNK_LOAD_RADIUS; z <= player_cz + CHUNK_LOAD_RADIUS; z++)
    {
        if (chunk_player_dist2(x, z, player_cx, player_cz) > CHUNK_LOAD_RADIUS2)
            continue;
        
        Chunk* c = map_get_chunk(x, z);

        int not_dirty  = c ? !c->is_dirty : 1;
        if (c && not_dirty)
            continue;
        int not_visible = !chunk_is_visible(x, z, cam->frustum_planes);
        int dist = chunk_player_dist2(x, z, player_cx, player_cz);
        
        // Visibility is more important than dirtiness, and dirtiness
        // is more important than distance
        int curr_score = ((not_visible << 24) | (not_dirty << 16)) + dist;
        if (curr_score <= best_score)
        {
            best_cx = x;
            best_cz = z;
            best_score = curr_score;
            found = 1;
        }
    }   

    if (found)
    {
        *best_x = best_cx;
        *best_z = best_cz;
        return true;
    }
    return false;
}

static void handle_workers(Camera* cam)
{
    for (int i = 0; i < map->num_workers; i++)
    {
        WorkerData* worker = &map->workers[i].data;
        mtx_lock(&worker->state_mtx);

        if (worker->state == WORKER_BUSY)
        {
            mtx_unlock(&worker->state_mtx);
            continue;
        }

        if (worker->state == WORKER_DONE)
        {
            worker->state = WORKER_IDLE;

            Chunk* c = worker->chunk;
            chunk_upload_mesh_to_gpu(c);

            mtx_unlock(&c->mtx);
        }

        if (worker->state == WORKER_IDLE)
        {
            int best_cx, best_cz;
            bool found = find_chunk_for_worker(cam, &best_cx, &best_cz);
            if (!found)
            {
                mtx_unlock(&worker->state_mtx);
                continue;
            }
            
            Chunk* c = map_get_chunk(best_cx, best_cz);
            if (c)
            {
                c->is_dirty = 0;
                worker->generate_terrain = 0;
            }
            else
            {
                c = chunk_init(best_cx, best_cz);
                hashmap_chunks_insert(map->chunks_active, c);
                worker->generate_terrain = 1;
            }
            
            mtx_lock(&c->mtx);
            worker->chunk = c;
            worker->state = WORKER_BUSY;

            mtx_unlock(&worker->state_mtx);
            cnd_signal(&worker->cond_var);
            continue;
        }
        
        mtx_unlock(&worker->state_mtx);
    }
}

static void load_chunk(int cx, int cz)
{
    Chunk* c = chunk_init(cx, cz);
    chunk_generate_terrain(c);
    chunk_generate_mesh(c);
    chunk_upload_mesh_to_gpu(c);

    hashmap_chunks_insert(map->chunks_active, c);
}

void map_force_chunks_near_player(Camera* cam)
{
    int const dist = 1;

    int player_cx = chunked_cam(cam->pos[0]);
    int player_cz = chunked_cam(cam->pos[2]);

    for (int dx = -dist; dx <= dist; dx++)
    for (int dz = -dist; dz <= dist; dz++)
    {
        int const cx = player_cx + dx;
        int const cz = player_cz + dz;

        if (!map_get_chunk(cx, cz))
            load_chunk(cx, cz);
    }
}

static void add_chunks_to_render_list(Camera* cam)
{
    MAP_FOREACH_ACTIVE_CHUNK_BEGIN(c)
        if (c->is_generated && chunk_is_visible(c->x, c->z, cam->frustum_planes))
            list_chunks_push_front(map->chunks_to_render, c);
    MAP_FOREACH_ACTIVE_CHUNK_END()
}

void map_update(Camera* cam)
{
    try_delete_far_chunks(cam);
    handle_workers(cam);
    map_force_chunks_near_player(cam);
    add_chunks_to_render_list(cam);
}

void map_set_seed(int new_seed)
{
    printf("Using seed: %d\n", new_seed);
    map->seed = new_seed;
}

void map_set_time(double new_time)
{
    glfwSetTime(DAY_LENGTH / 2 + new_time * DAY_LENGTH);
}

int map_get_seed()
{
    return map->seed;
}

int map_get_highest_block(int bx, int bz)
{
    Chunk* c = map_get_chunk(chunked_block(bx), chunked_block(bz));
    if (!c) return CHUNK_HEIGHT;

    int x = to_chunk_coord(bx);
    int z = to_chunk_coord(bz);

    int max_height = 0;
    for (int y = 0; y < CHUNK_HEIGHT; y++)
    {
        if (block_is_solid(c->blocks[XYZ(x, y, z)]))
            max_height = y;
    }

    return max_height;
}

void map_save()
{
    db_insert_map_info();
}

void map_exit()
{
    map_save();

    // Workers
    for (int i = 0; i < map->num_workers; i++)
        worker_destroy(&map->workers[i]);
    free(map->workers);

    // Chunk hashmaps and lists
    LinkedList_chunks* to_delete = list_chunks_create();
    MAP_FOREACH_ACTIVE_CHUNK_BEGIN(c)
    {
        list_chunks_push_back(to_delete, c);
    }
    MAP_FOREACH_ACTIVE_CHUNK_END()

    while (to_delete->size)
    {
        Chunk* c = list_chunks_pop_front(to_delete);
        chunk_delete(c);
    }

    hashmap_chunks_delete(map->chunks_active);
    list_chunks_delete(map->chunks_to_render);
    list_chunks_delete(to_delete);

    free(map);
    map = NULL;
}