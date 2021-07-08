#ifndef PLAYER_H_
#define PLAYER_H_

#include "camera.h"
#include "cglm/cglm.h"
#include "glad/glad.h"

typedef struct
{
    Camera* cam;

    unsigned char build_block;
    int pointing_at_block;
    ivec3 block_pointed_at;

    vec3 hitbox[2];

    int on_ground;
    int in_water;
    int is_sneaking;

    GLuint VAO_item;
    GLuint VBO_item;
}
Player;

Player* player_create();

void player_set_build_block(Player* p, int new_block);

void player_update(Player* p, double dt);

void player_handle_left_mouse_click(Player* p);

void player_handle_right_mouse_click(Player* p);

void player_render_item(Player* p);

void player_save(Player* p);

void player_exit(Player* p);

#endif