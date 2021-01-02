#ifndef WINDOW_H_
#define WINDOW_H_

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "camera.h"
#include "map.h"
#include "ui.h"
#include "player.h"

typedef struct
{
    Player* player;
    UI* ui;
}
GameObjects;

GLFWwindow* window_create();

#endif