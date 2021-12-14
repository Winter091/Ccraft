#include <camera/camera.h>

#include <stdlib.h>
#include <math.h>

#include <config.h>
#include <utils.h>
#include <map/block.h>
#include <window.h>

static void camera_framebuffer_size_change_callback(void* this_object, int new_width, int new_height)
{
    Camera* cam = (Camera*)this_object;
    camera_set_aspect_ratio(cam, new_width / new_height);
}

Camera* camera_create(vec3 pos, float pitch, float yaw, vec3 front)
{
    Camera* cam = malloc(sizeof(Camera));

    register_framebuffer_size_change_callback(cam, camera_framebuffer_size_change_callback);

    glm_vec3_copy(pos, cam->pos);
    glm_vec3_copy(pos, cam->prev_pos);
    // glm_vec3_fill(cam->speed, 0.0f);

    glm_vec3_copy(front, cam->front);
    my_glm_vec3_set(cam->up, 0.0f, 1.0f, 0.0f);

    cam->pitch = pitch;
    cam->yaw = yaw;

    cam->fov = FOV;
    cam->sens = MOUSE_SENS;

    cam->clip_near = BLOCK_SIZE / 10.0f;
    cam->clip_far = MAX((CHUNK_RENDER_RADIUS * 1.2f) * CHUNK_SIZE,
                        512 * BLOCK_SIZE);
    cam->aspect_ratio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    glm_look(cam->pos, cam->front, cam->up, cam->view_matrix);
    glm_perspective(glm_rad(cam->fov),
                    (float)WINDOW_WIDTH / WINDOW_HEIGHT,
                    cam->clip_near, cam->clip_far, cam->proj_matrix);
    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);
    glm_mat4_copy(cam->view_matrix, cam->prev_view_matrix);
    
    return cam;
}

void camera_update_matrices(Camera* cam)
{
    glm_mat4_copy(cam->view_matrix, cam->prev_view_matrix);
    glm_look(cam->pos, cam->front, cam->up, cam->view_matrix);
    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);

    glm_frustum_planes(cam->vp_matrix, cam->frustum_planes);
}

void camera_set_aspect_ratio(Camera* cam, float new_ratio)
{
    cam->aspect_ratio = new_ratio;
    glm_perspective_resize(new_ratio, cam->proj_matrix);
    glm_frustum_planes(cam->vp_matrix, cam->frustum_planes);
}

void camera_set_fov(Camera* cam, int new_fov)
{
    cam->fov = new_fov;
    glm_perspective(
        glm_rad(new_fov), cam->aspect_ratio,
        cam->clip_near, cam->clip_far, cam->proj_matrix
    );
}