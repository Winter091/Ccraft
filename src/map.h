#ifndef MAP_H_
#define MAP_H_

#include "chunk.h"
#include "linked_list.h"
#include "hashmap.h"
#include "camera.h"

LINKEDLIST_DEFINITION(Chunk*, chunks);
HASHMAP_DEFINITION(Chunk*, chunks);

typedef struct
{
    HashMap_chunks*    chunks_active;
    //LinkedList_chunks* chunks_to_load;
    //LinkedList_chunks* chunks_to_unload;
    //LinkedList_chunks* chunks_to_rebuild;
    LinkedList_chunks* chunks_to_render;

    GLuint shader_chunks;

    GLuint texture_blocks;
}
Map;

void map_init();
void map_update(Camera* cam);
void map_render_chunks(Camera* cam);
void map_handle_left_mouse_click(Camera* cam);
void map_handle_right_mouse_click(Camera* cam);

#endif