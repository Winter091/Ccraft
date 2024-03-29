#ifndef MAP_H_
#define MAP_H_

#include <glad/glad.h>

#include <linked_list.h>
#include <hashmap.h>
#include <map/chunk.h>
#include <camera/camera.h>
#include <player/player.h>

void map_init();

void map_update(Camera* cam);

void map_render_sun_moon(Camera* cam);

void map_render_sky(Camera* cam);

void map_render_chunks(Camera* cam, mat4 near_shadowmap_mat, mat4 far_shadowmap_mat);

void map_render_chunks_raw(vec4 frustum_planes[6]);

void map_force_chunks_near_player(vec3 curr_pos);

void map_set_block(int x, int y, int z, unsigned char block);

void map_set_seed(int new_seed);

void map_set_time(double new_time);

unsigned char map_get_block(int x, int y, int z);

double map_get_time();

int map_get_seed();

float map_get_blocks_light();

int map_get_highest_block(int x, int z);

void map_get_light_dir(vec3 res);

void map_save();

void map_free();

#endif