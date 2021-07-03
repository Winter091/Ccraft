#ifndef MAP_H_
#define MAP_H_

#include "chunk.h"
#include "linked_list.h"
#include "hashmap.h"
#include "camera.h"
#include "player.h"
#include "glad/glad.h"

void map_init();

void map_update(Camera* cam);

void map_render_sun_moon(Camera* cam);
void map_render_sky(Camera* cam);
void map_render_chunks(Camera* cam);
void map_force_chunks_near_player(Camera* cam);

void map_set_block(int x, int y, int z, unsigned char block);
void map_set_seed(int new_seed);
void map_set_time(double new_time);

unsigned char map_get_block(int x, int y, int z);
double map_get_time();
int map_get_seed();
int map_get_highest_block(int x, int z);

void map_save();
void map_exit();

#endif