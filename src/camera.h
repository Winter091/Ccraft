#ifndef CAMERA_H_
#define CAMERA_H_

#include "cglm/cglm.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

typedef struct
{
    int is_active;
    int is_fly_mode;
    int is_first_frame;

    vec3 pos;
    vec3 prev_pos;
    vec3 speed;

    vec3 front;
    vec3 up;

    double pitch;
    double yaw;

    int fov;
    float sens;
    float fly_speed;

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

void camera_update_view_dir(Camera* cam);

void camera_update_parameters(Camera* cam, float dt);

void camera_update_matrices(Camera* cam);

void camera_set_aspect_ratio(Camera* cam, float new_ratio);

void camera_set_fov(Camera* cam, int new_fov);

#endif