#include <window.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <config.h>
#include <framebuffer.h>

Window* g_window;

typedef struct
{
    void* object;
    void* callback_func;
}
UserCallbackEntry;


#define MAX_CALLBACKS 16

typedef struct
{
    UserCallbackEntry entries[MAX_CALLBACKS];
    size_t size;
}
UserCallbackArray;


static UserCallbackArray framebuffer_callbacks;
static UserCallbackArray keyboard_key_callbacks;
static UserCallbackArray mouse_button_callbacks;
static UserCallbackArray mouse_scroll_callbacks;


void register_framebuffer_size_change_callback(void* this_object, on_framebuffer_size_change user_callback)
{
    if (framebuffer_callbacks.size == MAX_CALLBACKS - 1)
        assert(false && "Max callbacks number is reached");
    
    UserCallbackEntry* curr_entry = &framebuffer_callbacks.entries[framebuffer_callbacks.size++];
    curr_entry->object = this_object;
    curr_entry->callback_func = user_callback;
}

void register_keyboard_key_press_callback(void* this_object, on_keyboard_key_press user_callback)
{
    if (keyboard_key_callbacks.size == MAX_CALLBACKS - 1)
        assert(false && "Max callbacks number is reached");
    
    UserCallbackEntry* curr_entry = &keyboard_key_callbacks.entries[keyboard_key_callbacks.size++];
    curr_entry->object = this_object;
    curr_entry->callback_func = user_callback;
}

void register_mouse_button_key_press_callback(void* this_object, on_mouse_button_key_press user_callback)
{
    if (mouse_button_callbacks.size == MAX_CALLBACKS - 1)
        assert(false && "Max callbacks number is reached");
    
    UserCallbackEntry* curr_entry = &mouse_button_callbacks.entries[mouse_button_callbacks.size++];
    curr_entry->object = this_object;
    curr_entry->callback_func = user_callback;
}

void register_mouse_scroll_callback(void* this_object, on_mouse_scroll user_callback)
{
    if (mouse_scroll_callbacks.size == MAX_CALLBACKS - 1)
        assert(false && "Max callbacks number is reached");
    
    UserCallbackEntry* curr_entry = &mouse_scroll_callbacks.entries[mouse_scroll_callbacks.size++];
    curr_entry->object = this_object;
    curr_entry->callback_func = user_callback;
}

static void set_focused(int is_focused)
{
    g_window->is_focused = is_focused;
    
    const int cursor_mode = is_focused ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
    glfwSetInputMode(g_window->glfw, GLFW_CURSOR, cursor_mode);
}

static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    g_window->width  = width;
    g_window->height = height;
    glViewport(0, 0, width, height);

    for (int i = 0; i < framebuffer_callbacks.size; i++)
    {
        UserCallbackEntry* entry = &framebuffer_callbacks.entries[i];
        ((on_framebuffer_size_change)entry->callback_func)(entry->object, width, height);
    }
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!g_window->is_focused)
        return;
    
    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            set_focused(0);
    };
    
    for (int i = 0; i < keyboard_key_callbacks.size; i++)
    {
        UserCallbackEntry* entry = &keyboard_key_callbacks.entries[i];
        ((on_keyboard_key_press)entry->callback_func)(entry->object, key, action);
    }
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) 
        set_focused(1);

    if (!g_window->is_focused)
        return;
    
    for (int i = 0; i < mouse_button_callbacks.size; i++)
    {
        UserCallbackEntry* entry = &mouse_button_callbacks.entries[i];
        ((on_mouse_button_key_press)entry->callback_func)(entry->object, button, action);
    }
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (!g_window->is_focused)
        return;
    
    for (int i = 0; i < mouse_scroll_callbacks.size; i++)
    {
        UserCallbackEntry* entry = &mouse_scroll_callbacks.entries[i];
        ((on_mouse_scroll)entry->callback_func)(entry->object, (float)xoffset, (float)yoffset);
    }
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
    glfwSetFramebufferSizeCallback(g_window->glfw, glfw_framebuffer_size_callback);
    glfwSetKeyCallback(g_window->glfw, glfw_key_callback);
    glfwSetMouseButtonCallback(g_window->glfw, glfw_mouse_button_callback);
    glfwSetScrollCallback(g_window->glfw, glfw_scroll_callback);

    g_window->fb = NULL;

    g_window->width  = WINDOW_WIDTH;
    g_window->height = WINDOW_HEIGHT;

    g_window->is_focused = 1;
}

void window_init_fb()
{
    g_window->fb = framebuffers_create(WINDOW_WIDTH, WINDOW_HEIGHT);
}

int window_is_key_pressed(int glfw_keycode)
{
    return glfwGetKey(g_window->glfw, glfw_keycode) == GLFW_PRESS;
}

void window_update_title_fps()
{
    const float update_interval_secs = 0.5f;
    
    static double last_time = -1.0;
    if (last_time < 0)
        last_time = glfwGetTime();

    static int num_frames = 0;
    num_frames++;

    double curr_time = glfwGetTime();
    if (curr_time - last_time >= update_interval_secs)
    {
        int fps = (int)lroundf((float)num_frames / update_interval_secs);

        char title[128];
        sprintf(title, "%s - %d FPS", WINDOW_TITLE, fps);
        glfwSetWindowTitle(g_window->glfw, title);
        
        num_frames = 0;
        last_time = curr_time;
    }
}

void window_poll_events()
{
    glfwPollEvents();
}

void window_free()
{
    framebuffers_destroy(g_window->fb);
    
    glfwDestroyWindow(g_window->glfw);
    glfwTerminate();

    free(g_window);
    g_window = NULL;
}