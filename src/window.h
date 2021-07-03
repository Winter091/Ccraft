#ifndef WINDOW_H_
#define WINDOW_H_

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "camera.h"
#include "map.h"
#include "ui.h"
#include "player.h"

// Globally accessible window width and height
extern int curr_window_w;
extern int curr_window_h;

// At one point of development I thought there
// will be a lot of objects, but it looks like I
// was a bit wrong; I don't want to make player
// struct static as every other structure, so 
// here we are, having only one object
typedef struct
{
    Player* player;
}
GameObjectRefs;

GLFWwindow* window_create();

#endif