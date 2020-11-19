#ifndef WINDOW_H_
#define WINDOW_H_

#include "GLFW/glfw3.h"

#include "camera.h"

typedef struct
{
    Camera* cam;
}
GameObjects;

GLFWwindow* window_create();

#endif