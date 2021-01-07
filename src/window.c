#include "glad/glad.h"

#include "window.h"

#include "stdio.h"
#include "stdlib.h"

#include "config.h"
#include "framebuffer.h"

int window_w = WINDOW_WIDTH;
int window_h = WINDOW_HEIGHT;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    GameObjects* game = glfwGetWindowUserPointer(window);

    camera_set_aspect_ratio(game->player->cam, (float)width / height);
    ui_update_aspect_ratio(game->ui, (float)width / height);
    framebuffer_rebuild(width, height);

    window_w = width;
    window_h = height;

    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE)
        return;

    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
    };
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    GameObjects* game = glfwGetWindowUserPointer(window);
    if (!game->player->cam->active)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        return;
    }

    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            player_handle_left_mouse_click(game->player);
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            player_handle_right_mouse_click(game->player);
            break;
    };
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    GameObjects* game = glfwGetWindowUserPointer(window);
    if (yoffset > 0)
        player_set_build_block(game->player, game->player->build_block + 1);
    else
        player_set_build_block(game->player, game->player->build_block - 1);
}

GLFWwindow* window_create()
{
    if (!glfwInit())
    {
        fprintf(stderr, "GLFW failed to init.\n");
        exit(EXIT_FAILURE);
    }

    // require to use particular version of OpenGL
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR_REQUIRED);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR_REQUIRED);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = FULLSCREEN ? glfwGetPrimaryMonitor() : NULL;

    // setup windowed full screen mode
    if (monitor)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
    }

    // will fail if OpenGL version is not supported
    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        WINDOW_TITLE, monitor, NULL
    );

    if (!window)
    {
        fprintf(stderr, "GLFW window failed to init.\n");
        fprintf(stderr, 
            "Probably, the required version of OpenGL is not supported:\n"
        );
        fprintf(stderr, "OpenGL %d.%d\n", 
            OPENGL_VERSION_MAJOR_REQUIRED, OPENGL_VERSION_MINOR_REQUIRED
        );
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(VSYNC);

    // set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}