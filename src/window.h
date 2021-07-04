#ifndef WINDOW_H_
#define WINDOW_H_

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "camera.h"
#include "map.h"
#include "ui.h"
#include "player.h"
#include "framebuffer.h"

typedef struct
{
    GLFWwindow* glfw;
    Framebuffers* fb;
    
    int width;
    int height;
}
Window;

typedef struct
{
    Player* player;
    Window* window;
}
GameObjectRefs;

extern Window* g_window;

void window_init();
void window_init_fb();
void window_destroy();

#endif