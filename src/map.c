#include "glad/glad.h"
#include "map.h"

#include "stdlib.h"
#include "limits.h"
#include "shader.h"
#include "texture.h"
#include "fastnoiselite.h"
#include "utils.h"
#include "math.h"
#include "db.h"
#include "block.h"

LINKEDLIST_DEFINITION(Chunk*, chunks);
HASHMAP_DEFINITION(Chunk*, chunks);

LINKEDLIST_IMPLEMENTATION(Chunk*, chunks);
HASHMAP_IMPLEMENTATION(Chunk*, chunks, chunk_hash_func);

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
    HashMap_chunks*    chunks_active;
    //LinkedList_chunks* chunks_to_load;
    //LinkedList_chunks* chunks_to_unload;
    //LinkedList_chunks* chunks_to_rebuild;
    LinkedList_chunks* chunks_to_render;

    GLuint VAO_skybox;
    GLuint VBO_skybox;

    GLuint VAO_sun_moon;
    GLuint VBO_sun_moon;
}
Map;

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
            map_update_chunk_buffer(neighs[i], 0);
    }
}

static void map_load_chunk(int chunk_x, int chunk_z)
{
    Chunk* c = chunk_create(chunk_x, chunk_z);
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
            // that list is being cleared every frame
            if (c)
            {
                if (chunk_is_visible(x, z, cam->frustum_planes))
                    list_chunks_push_front(map->chunks_to_render, c);
            }

            // find the closest visible chunk that is not loaded yet
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
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    map->VAO_sun_moon = opengl_create_vao();
    map->VBO_sun_moon = opengl_create_vbo_quad();
    opengl_vbo_layout(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    opengl_vbo_layout(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));

#if USE_DATABASE
    db_get_map_info();
#endif
}

// [0.0 - 1.0)
double map_get_time()
{
#if DISABLE_TIME_FLOW
    return 0.0;
#else
    return 0.5 + remainder(glfwGetTime(), DAY_LENGTH) / (double)DAY_LENGTH;
#endif
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
    static vec3 day_color = {0.5f, 0.6f, 0.7f}; 
    static vec3 evening_color = {1.0f, 0.9f, 0.7f}; 
    static vec3 night_color = {0.2f, 0.2f, 0.2f}; 
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

    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    LIST_FOREACH_CHUNK_BEGIN(map->chunks_to_render, c)
    {
        glBindVertexArray(c->VAO_land);
        glDrawArrays(GL_TRIANGLES, 0, c->vertex_land_count);
    }
    LIST_FOREACH_CHUNK_END()

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    LIST_FOREACH_CHUNK_BEGIN(map->chunks_to_render, c)
    {
        glBindVertexArray(c->VAO_water);
        glDrawArrays(GL_TRIANGLES, 0, c->vertex_water_count);
    }
    LIST_FOREACH_CHUNK_END()
    glDepthMask(GL_TRUE);

    list_chunks_clear(map->chunks_to_render);
}

unsigned char map_get_block(int x, int y, int z)
{
    Chunk* c = map_get_chunk(chunked(x), chunked(z));
    if (!c || !c->is_loaded || y >= CHUNK_HEIGHT || y < 0) 
        return BLOCK_AIR;

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

void map_set_seed(int new_seed)
{
    printf("Using seed: %d\n", new_seed);
    noise_set_seed(new_seed);
}

void map_set_time(double new_time)
{
    glfwSetTime(DAY_LENGTH / 2 + new_time * DAY_LENGTH);
}

int map_get_seed()
{
    return noise_get_seed();
}

void map_save()
{
    db_insert_map_info();
}