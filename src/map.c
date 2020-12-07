#include "glad/glad.h"
#include "map.h"

#include "stdlib.h"
#include "limits.h"

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

    return map;
}

void map_update(Map* map, Camera* cam)
{
    //float curr_time = glfwGetTime();
    
    map_unload_far_chunks(map, cam);
    map_load_chunks(map, cam);

    //printf("%.6f\n", glfwGetTime() - curr_time);
}

void map_render_chunks(Map* map, vec4 cam_frustum_planes[6])
{    
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

static void map_delete_block(Map* map, int x, int y, int z)
{
    Chunk* c = map_get_chunk(map, chunked(x), chunked(z));
    if (!c) return;

    // get block coord in 'chunk coord space'
    int block_x = x % CHUNK_WIDTH;
    int block_z = z % CHUNK_WIDTH;
    if (block_x < 0) block_x += CHUNK_WIDTH;
    if (block_z < 0) block_z += CHUNK_WIDTH;

    c->blocks[XYZ(block_x, y, block_z)] = BLOCK_AIR;

    // possible optimization: update neighbour
    // only if the block destroyed is on the border
    // of chunk
    map_update_chunk_buffer(map, c, 1);
}

// squared distance between 2 points in space;
// sqrt() function is quite expensive
static int dist2(int a, int b, int c, int x, int y, int z)
{
    return (a - x) * (a - x) + (b - y) * (b - y) + (c - z) * (c - z);
}

void map_handle_left_mouse_click(Map* map, Camera* cam)
{
    // position of camera in blocks
    int cam_x = cam->pos[0] / BLOCK_SIZE;
    int cam_y = cam->pos[1] / BLOCK_SIZE;
    int cam_z = cam->pos[2] / BLOCK_SIZE;

    // best_* will store the block to destroy,
    // if it's found
    int best_x, best_y, best_z;
    int best_dist = INT_MAX;

    // iterate over each block around player
    for (int y = cam_y - BLOCK_BREAK_RADIUS; y <= cam_y + BLOCK_BREAK_RADIUS; y++)
    {
        if (y < 0) continue;

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
        return;
        
    //printf("%d %d %d\n", best_x, best_y, best_z);
    map_delete_block(map, best_x, best_y, best_z);
}