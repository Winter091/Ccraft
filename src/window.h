#ifndef WINDOW_H_
#define WINDOW_H_

#include "GLFW/glfw3.h"

#include "camera.h"
#include "map.h"
#include "ui.h"

typedef struct
{
    Camera* cam;
    Map* map;
    UI* ui;
}
GameObjects;

GLFWwindow* window_create();

#endif