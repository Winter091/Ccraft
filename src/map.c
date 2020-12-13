#include "glad/glad.h"
#include "map.h"

#include "stdlib.h"
#include "limits.h"
#include "shader.h"
#include "texture.h"
#include "perlin_noise.h"

LINKEDLIST_IMPLEMENTATION(Chunk*, chunks);
HASHMAP_IMPLEMENTATION(Chunk*, chunks, chunk_hash_func);

static Chunk* map_get_chunk(Map* map, int chunk_x, int chunk_z)
{
    uint32_t index = chunk_hash_func2(chunk_x, chunk_z) % map->chunks_active->array_size;
    LinkedListMap_chunks* bucket = map->chunks_active->array[index];
    
    LinkedListNodeMap_chunks* node = bucket->head;

    while (node && !(node->data->x == chunk_x && node->data->z == chunk_z))
        node = node->ptr_next;

    return node ? node->data : NULL;
}

static void map_update_chunk_buffer(Map* map, Chunk* c, int update_neighbours)
{
    Chunk* left  = map_get_chunk(map, c->x - 1, c->z);
    Chunk* right = map_get_chunk(map, c->x + 1, c->z);
    Chunk* front = map_get_chunk(map, c->x, c->z + 1);
    Chunk* back  = map_get_chunk(map, c->x, c->z - 1);

    chunk_update_buffer(c, left, right, front, back);

    if (update_neighbours)
    {
        if (left)  map_update_chunk_buffer(map, left,  0);
        if (right) map_update_chunk_buffer(map, right, 0);
        if (front) map_update_chunk_buffer(map, front, 0);
        if (back)  map_update_chunk_buffer(map, back,  0);
    }
}

static void map_load_chunk(Map* map, int chunk_x, int chunk_z)
{
    Chunk* c = chunk_init(chunk_x, chunk_z);
    chunk_generate(c);
    hashmap_chunks_insert(map->chunks_active, c);
    map_update_chunk_buffer(map, c, 1);
    //printf("Loaded %d %d\n", chunk_x, chunk_z);
}

static void map_delete_chunk(Map* map, int chunk_x, int chunk_z)
{
    Chunk* c = map_get_chunk(map, chunk_x, chunk_z);
    if (c)
    {
        hashmap_chunks_remove(map->chunks_active, c);
        chunk_delete(c);
        //printf("Deleted %d %d\n", chunk_x, chunk_z);
    }
}

static void map_unload_far_chunks(Map* map, Camera* cam)
{
    int cam_chunk_x = cam->pos[0] / CHUNK_SIZE;
    int cam_chunk_z = cam->pos[2] / CHUNK_SIZE;
    
    for (int i = 0; i < map->chunks_active->array_size; i++)
    {  
        LinkedListNodeMap_chunks* node = map->chunks_active->array[i]->head;
        for ( ; node; node = node->ptr_next)
        {
            Chunk* c = node->data;
            //if (chunk_dist_to_player(c->x, c->z, cam_chunk_x, cam_chunk_z) > CHUNK_UNLOAD_RADIUS)
            if (abs(c->x - cam_chunk_x) > CHUNK_UNLOAD_RADIUS || abs(c->z - cam_chunk_z) > CHUNK_UNLOAD_RADIUS)
            {                
                map_delete_chunk(map, c->x, c->z);
            }
        }
    }
}

static void map_load_chunks(Map* map, Camera* cam)
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
            Chunk* c = map_get_chunk(map, x, z);

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
        map_load_chunk(map, best_chunk_x, best_chunk_z);
}

Map* map_create()
{
    Map* map = malloc(sizeof(Map));

    map->chunks_active     = hashmap_chunks_create(CHUNK_RENDER_RADIUS * CHUNK_RENDER_RADIUS * 1.2f);
    //map->chunks_to_load    = list_chunks_create();
    //map->chunks_to_unload  = list_chunks_create();
    //map->chunks_to_rebuild = list_chunks_create();
    map->chunks_to_render  = list_chunks_create();

    map->shader_chunks = create_shader_program(
        "shaders/chunk_vertex.glsl",
        "shaders/chunk_fragment.glsl"
    );

    map->shader_lines = create_shader_program(
        "shaders/line_vertex.glsl",
        "shaders/line_fragment.glsl"
    );

    map->texture_blocks = array_texture_create("textures/blocks.png");
    if (!map->texture_blocks)
    {
        fprintf(stderr, "Texture was not loaded!\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // set world seed
    *perlin2d_get_world_seed() = rand();

    return map;
}

void map_render_chunks(Map* map, Camera* cam)
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

void map_render_wireframe(Map* map, Camera* cam)
{
    // Each frame new buffer is generated, the better 
    // solution is to just offset data using camera
    // vp matrix
    
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    float x = cam->active_block[0] * BLOCK_SIZE;
    float y = cam->active_block[1] * BLOCK_SIZE;
    float z = cam->active_block[2] * BLOCK_SIZE;
    float bs = BLOCK_SIZE;

    float offset = 0.001f * BLOCK_SIZE;
    float vertices[] = {
        x - offset, y - offset, z - offset,
        x + bs + offset, y - offset, z - offset,

        x + bs + offset, y - offset, z - offset,
        x + bs + offset, y + bs + offset, z - offset,

        x + bs + offset, y + bs + offset, z - offset,
        x - offset, y + bs + offset, z - offset,

        x - offset, y + bs + offset, z - offset,
        x - offset, y - offset, z - offset,



        x - offset, y - offset, z + bs + offset,
        x + bs + offset, y - offset, z + bs + offset,

        x + bs + offset, y - offset, z + bs + offset,
        x + bs + offset, y + bs + offset, z + bs + offset,

        x + bs + offset, y + bs + offset, z + bs + offset,
        x - offset, y + bs + offset, z + bs + offset,

        x - offset, y + bs + offset, z + bs + offset,
        x - offset, y - offset, z + bs + offset,



        x - offset, y - offset, z + bs + offset,
        x - offset, y - offset, z - offset,

        x + bs + offset, y - offset, z + bs + offset,
        x + bs + offset, y - offset, z - offset,

        x + bs + offset, y + bs + offset, z + bs + offset,
        x + bs + offset, y + bs + offset, z - offset,

        x - offset, y + bs + offset, z + bs + offset,
        x - offset, y + bs + offset, z - offset
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(map->shader_lines);
    shader_set_mat4(map->shader_lines, "mvp_matrix", cam->vp_matrix);

    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_INVERT);
    glLineWidth(2);
    glDrawArrays(GL_LINES, 0, 24);
    glDisable(GL_COLOR_LOGIC_OP);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

// return chunk coordinate block exists in
static int chunked(int a)
{
    if (a >= 0) return a / CHUNK_WIDTH;
    return (a + 1) / CHUNK_WIDTH - 1;
}

static unsigned char map_get_block(Map* map, int x, int y, int z)
{
    int chunk_x = chunked(x);
    int chunk_z = chunked(z);

    int block_x = x % CHUNK_WIDTH;
    int block_z = z % CHUNK_WIDTH;
    if (block_x < 0) block_x += CHUNK_WIDTH;
    if (block_z < 0) block_z += CHUNK_WIDTH;

    Chunk* c = map_get_chunk(map, chunk_x, chunk_z);
    if (!c || !c->is_loaded) 
        return 0;

    //printf("returning block %d from chunk %d %d\n", c->blocks[XYZ(block_x, y, block_z)], chunk_x, chunk_z);
    return c->blocks[XYZ(block_x, y, block_z)];
}

static void map_set_block(Map* map, int x, int y, int z, unsigned char block)
{
    int chunk_x = chunked(x);
    int chunk_z = chunked(z);
    
    Chunk* c = map_get_chunk(map, chunk_x, chunk_z);
    if (!c) return;

    // get block coord in 'chunk coord space' ([0 - CHUNK_WIDTH))
    int block_x = x % CHUNK_WIDTH;
    int block_z = z % CHUNK_WIDTH;
    if (block_x < 0) block_x += CHUNK_WIDTH;
    if (block_z < 0) block_z += CHUNK_WIDTH;

    c->blocks[XYZ(block_x, y, block_z)] = block;

    // update chunk buffer (and neighbours, if needed)
    map_update_chunk_buffer(map, c, 0);
    
    if (block_x == 0)
    {
        Chunk* neigh = map_get_chunk(map, chunk_x - 1, chunk_z);
        if (neigh) map_update_chunk_buffer(map, neigh, 0);
    }
    else if (block_x == CHUNK_WIDTH - 1)
    {
        Chunk* neigh = map_get_chunk(map, chunk_x + 1, chunk_z);
        if (neigh) map_update_chunk_buffer(map, neigh, 0);
    }
    if (block_z == 0)
    {
        Chunk* neigh = map_get_chunk(map, chunk_x, chunk_z - 1);
        if (neigh) map_update_chunk_buffer(map, neigh, 0);
    }
    else if (block_z == CHUNK_WIDTH - 1)
    {
        Chunk* neigh = map_get_chunk(map, chunk_x, chunk_z + 1);
        if (neigh) map_update_chunk_buffer(map, neigh, 0);
    }
}

// squared distance between 2 points in space;
// sqrt() function is quite expensive
static int dist2(int a, int b, int c, int x, int y, int z)
{
    return (a - x) * (a - x) + (b - y) * (b - y) + (c - z) * (c - z);
}

void map_handle_left_mouse_click(Map* map, Camera* cam)
{
    if (!cam->active_block_present)
        return;

    // if camera looks at block nearby, remove the block
    map_set_block(
        map, 
        cam->active_block[0], 
        cam->active_block[1], 
        cam->active_block[2],
        BLOCK_AIR
    );
    cam->active_block_present = 0;
}

void find_best_spot_to_place_block(
    Map* map, Camera* cam, int x, int y, int z, 
    int* best_x, int* best_y, int* best_z, int* best_dist
)
{
    // position of camera in blocks
    int cam_x = cam->pos[0] / BLOCK_SIZE;
    int cam_y = cam->pos[1] / BLOCK_SIZE;
    int cam_z = cam->pos[2] / BLOCK_SIZE;
    
    if (camera_looks_at_block(cam, x, y, z) && map_get_block(map, x, y, z) == BLOCK_AIR)
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

void map_handle_right_mouse_click(Map* map, Camera* cam)
{
    if (!cam->active_block_present)
        return;
    
    int x = cam->active_block[0];
    int y = cam->active_block[1];
    int z = cam->active_block[2];

    int best_x, best_y, best_z;
    int best_dist = INT_MAX;

    // 6 potential spots around active block
    find_best_spot_to_place_block(map, cam, x - 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(map, cam, x + 1, y, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(map, cam, x, y - 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(map, cam, x, y + 1, z, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(map, cam, x, y, z - 1, &best_x, &best_y, &best_z, &best_dist);
    find_best_spot_to_place_block(map, cam, x, y, z + 1, &best_x, &best_y, &best_z, &best_dist);

    if (best_dist != INT_MAX)
    {
        map_set_block(map, best_x, best_y, best_z, BLOCK_DIRT);
    }
}

static void map_set_camera_active_block(Map* map, Camera* cam)
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
                    unsigned char block = map_get_block(map, x, y, z);
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
        cam->active_block_present = 0;
        return;
    }

    cam->active_block_present = 1;
    cam->active_block[0] = best_x;
    cam->active_block[1] = best_y;
    cam->active_block[2] = best_z;
}

void map_update(Map* map, Camera* cam)
{
    //float curr_time = glfwGetTime();
    
    map_unload_far_chunks(map, cam);
    map_load_chunks(map, cam);
    map_set_camera_active_block(map, cam);

    //printf("%.6f\n", glfwGetTime() - curr_time);
}