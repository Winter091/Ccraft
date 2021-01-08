#ifndef WINDOW_H_
#define WINDOW_H_

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "camera.h"
#include "map.h"
#include "ui.h"
#include "player.h"

extern int window_w;
extern int window_h;

typedef struct
{
    Player* player;
}
GameObjects;

GLFWwindow* window_create();

#endif