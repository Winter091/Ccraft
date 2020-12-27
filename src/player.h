#ifndef PLAYER_H_
#define PLAYER_H_

#include "camera.h"
#include "cglm/cglm.h"

typedef struct
{
    Camera* cam;
    unsigned char build_block;
    int pointing_at_block;
    ivec3 block_pointed_at;
}
Player;

Player* player_create(vec3 pos, vec3 dir);
void player_set_build_block(Player* p, int new_block);
void player_update(Player* p, GLFWwindow* window, double dt);

void player_handle_left_mouse_click(Player* p);
void player_handle_right_mouse_click(Player* p);

#endif