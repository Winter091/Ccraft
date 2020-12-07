#ifndef WINDOW_H_
#define WINDOW_H_

#include "GLFW/glfw3.h"

#include "camera.h"
#include "map.h"

typedef struct
{
    Camera* cam;
    Map* map;
}
GameObjects;

GLFWwindow* window_create();

#endif