#ifndef PLAYER_CONTROLLER_H_
#define PLAYER_CONTROLLER_H_

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <player/player.h>
#include <map/block.h>
#include <map/map.h>

typedef struct
{
    Player* player;

    int is_first_frame;
    int is_fly_mode;
    int is_controlling;
    
    double last_mouse_x;
    double last_mouse_y;
    float mouse_sens;
}
PlayerController;

PlayerController* playercontroller_create(Player* p);

void playercontroller_do_control(PlayerController* pc);

void playercontroller_destroy(PlayerController* pc);
#endif