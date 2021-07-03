#include "window.h"

#include "stdio.h"
#include "stdlib.h"

#include "config.h"
#include "framebuffer.h"

int curr_window_w;
int curr_window_h;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    GameObjectRefs* game = glfwGetWindowUserPointer(window);
    Player* p = game->player;

    camera_set_aspect_ratio(p->cam, (float)width / height);
    ui_update_aspect_ratio((float)width / height);
    framebuffer_rebuild(width, height);

    curr_window_w = width;
    curr_window_h = height;

    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE)
        return;

    GameObjectRefs* game = glfwGetWindowUserPointer(window);
    Player* p = game->player;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            p->cam->active = 0;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
    };
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    GameObjectRefs* game = glfwGetWindowUserPointer(window);
    Player* p = game->player;

    if (!p->cam->active)
    {
        p->cam->active = 1;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        p->cam->mouse_last_x = x;
        p->cam->mouse_last_y = y;

        return;
    }

    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            player_handle_left_mouse_click(p);
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            player_handle_right_mouse_click(p);
            break;
    };
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    GameObjectRefs* game = glfwGetWindowUserPointer(window);
    Player* p = game->player;
    
    if (yoffset > 0)
        player_set_build_block(p, p->build_block + 1);
    else
        player_set_build_block(p, p->build_block - 1);
}

GLFWwindow* window_create()
{
    if (!glfwInit())
    {
        fprintf(stderr, "GLFW failed to init.\n");
        exit(EXIT_FAILURE);
    }

    // Require to use particular version of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR_REQUIRED);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR_REQUIRED);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#else
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
#endif

    int count;
    GLFWmonitor* monitor = FULLSCREEN ? glfwGetMonitors(&count)[1] : NULL;

    // Setup windowed full screen mode
    if (monitor)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
    }

    // Will fail if OpenGL version is not supported
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 
                                          WINDOW_TITLE, monitor, NULL);

    curr_window_w = WINDOW_WIDTH;
    curr_window_h = WINDOW_HEIGHT;

    if (!window)
    {
        fprintf(stderr, "GLFW window failed to init.\n");
        fprintf(stderr, "Probably, the required version of OpenGL is not supported:\n");
        fprintf(stderr, "OpenGL %d.%d\n", OPENGL_VERSION_MAJOR_REQUIRED, 
                OPENGL_VERSION_MINOR_REQUIRED);
        
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(VSYNC);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}