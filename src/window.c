#include "window.h"

#include "stdio.h"
#include "stdlib.h"

#include "config.h"
#include "framebuffer.h"

Window* g_window;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    GameObjectRefs* game = glfwGetWindowUserPointer(window);
    Player* p = game->player;

    camera_set_aspect_ratio(p->cam, (float)width / height);
    ui_update_aspect_ratio((float)width / height);
    framebuffers_rebuild(g_window->fb, width, height);

    g_window->width  = width;
    g_window->height = height;

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
            p->cam->is_active = 0;
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

    if (!p->cam->is_active)
    {
        p->cam->is_active = 1;
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

void window_init()
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

    GLFWmonitor* monitor = FULLSCREEN ? glfwGetPrimaryMonitor() : NULL;

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

    g_window = malloc(sizeof(Window));

    // Will fail if OpenGL version is not supported
    g_window->glfw = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 
                                      WINDOW_TITLE, monitor, NULL);
    
    if (!g_window->glfw)
    {
        fprintf(stderr, "GLFW window failed to init.\n");
        fprintf(stderr, "Probably, the required version of OpenGL is not supported:\n");
        fprintf(stderr, "OpenGL %d.%d\n", OPENGL_VERSION_MAJOR_REQUIRED, 
                OPENGL_VERSION_MINOR_REQUIRED);
        
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(g_window->glfw);
    glfwSetInputMode(g_window->glfw, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(VSYNC);

    // Set callbacks
    glfwSetFramebufferSizeCallback(g_window->glfw, framebuffer_size_callback);
    glfwSetKeyCallback(g_window->glfw, key_callback);
    glfwSetMouseButtonCallback(g_window->glfw, mouse_button_callback);
    glfwSetScrollCallback(g_window->glfw, scroll_callback);

    g_window->fb = NULL;

    g_window->width  = WINDOW_WIDTH;
    g_window->height = WINDOW_HEIGHT;
}

void window_init_fb()
{
    g_window->fb = framebuffers_init(WINDOW_WIDTH, WINDOW_HEIGHT);
}

int window_is_key_pressed(int glfw_keycode)
{
    return glfwGetKey(g_window->glfw, glfw_keycode) == GLFW_PRESS;
}

void window_destroy()
{
    framebuffers_destroy(g_window->fb);
    
    glfwDestroyWindow(g_window->glfw);
    glfwTerminate();

    free(g_window);
    g_window = NULL;
}