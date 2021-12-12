#ifndef WINDOW_H_
#define WINDOW_H_

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <camera/camera.h>
#include <map/map.h>
#include <ui.h>
#include <player/player.h>
#include <framebuffer.h>

typedef void (*on_framebuffer_size_change)(void* this_object, int new_width, int new_height);
typedef void (*on_keyboard_key_press)(void* this_object, int glfw_keycode, int glfw_action_code);
typedef void (*on_mouse_button_key_press)(void* this_object, int glfw_keycode, int glfw_action_code);
typedef void (*on_mouse_scroll)(void* this_object, float xoffset, float yoffset);

typedef struct
{
    GLFWwindow* glfw;
    Framebuffers* fb;
    
    int width;
    int height;

    int is_focused;
}
Window;

typedef struct
{
    Player* player;
}
GameObjectRefs;

extern Window* g_window;

void window_init();

void window_init_fb();

void window_poll_events();

int window_is_key_pressed(int glfw_keycode);

void window_update_title_fps();

void register_framebuffer_size_change_callback(void* this_object, on_framebuffer_size_change user_callback);
void register_keyboard_key_press_callback(void* this_object, on_keyboard_key_press user_callback);
void register_mouse_button_key_press_callback(void* this_object, on_mouse_button_key_press user_callback);
void register_mouse_scroll_callback(void* this_object, on_mouse_scroll user_callback);

void window_free();

#endif