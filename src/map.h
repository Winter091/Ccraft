#ifndef MAP_H_
#define MAP_H_

#include "chunk.h"
#include "linked_list.h"
#include "hashmap.h"
#include "camera.h"
#include "player.h"
#include "glad/glad.h"

LINKEDLIST_DEFINITION(Chunk*, chunks);
HASHMAP_DEFINITION(Chunk*, chunks);

#define MAP_FOREACH_ACTIVE_CHUNK_BEGIN(CHUNK_NAME)\
for (int i = 0; i < map->chunks_active->array_size; i++)\
{\
    LinkedListNodeMap_chunks* node = map->chunks_active->array[i]->head;\
    for ( ; node; node = node->ptr_next)\
    {\
        Chunk* CHUNK_NAME = node->data;\

#define MAP_FOREACH_ACTIVE_CHUNK_END() }}

#define LIST_FOREACH_CHUNK_BEGIN(LIST, CHUNK_NAME)\
LinkedListNode_chunks* node = LIST->head;\
for ( ; node; node = node->ptr_next)\
{\
    Chunk* c = node->data;\

#define LIST_FOREACH_CHUNK_END() }

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

void map_init();
void map_update(Camera* cam);
void map_render_sun_moon(Camera* cam);
void map_render_sky(Camera* cam);
void map_render_chunks(Camera* cam);
unsigned char map_get_block(int x, int y, int z);
void map_set_block(int x, int y, int z, unsigned char block);

#endif