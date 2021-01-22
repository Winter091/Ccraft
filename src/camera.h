#ifndef CAMERA_H_
#define CAMERA_H_

#include "cglm/cglm.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

typedef struct
{
    int active;
    int fly_mode;

    vec3 pos;
    vec3 prev_pos;

    vec3 frame_motion;
    vec2 motion_horizontal;
    float motion_vertical;

    vec3 front;
    vec3 up;

    float pitch;
    float yaw;

    int fov;
    float sens;
    float fly_speed;

    int first_frame;
    float mouse_last_x;
    float mouse_last_y;

    float clip_near;
    float clip_far;
    float aspect_ratio;

    vec4 frustum_planes[6];
    mat4 view_matrix;
    mat4 proj_matrix;
    mat4 vp_matrix;
    mat4 prev_view_matrix;
}
Camera;

Camera* camera_create();
int camera_looks_at_block(Camera* cam, int x, int y, int z, unsigned char block_type);

void camera_update_view_dir(Camera* cam, GLFWwindow* window);
void camera_update_parameters(Camera* cam, GLFWwindow* window, double dt);
void camera_update_matrices(Camera* cam);

void camera_set_aspect_ratio(Camera* cam, float new_ratio);
void camera_set_fov(Camera* cam, int new_fov);

#endif