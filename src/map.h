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
}
Map;

Map* map_create(vec3 cam_pos);
void map_update(Map* map, Camera* cam);
void map_render_chunks(Map* map, vec4 cam_frustum_planes[6]);

#endif