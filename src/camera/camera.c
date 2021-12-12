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

// ray - aabb hit detection, see
// https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
int camera_looks_at_block(vec3 pos, vec3 front, int x, int y, int z, unsigned char block_type)
{
    vec3 min = { x * BLOCK_SIZE, y * BLOCK_SIZE, z * BLOCK_SIZE };
    vec3 max = { min[0] + BLOCK_SIZE, min[1] + BLOCK_SIZE, min[2] + BLOCK_SIZE };

    // make hitbox smaller
    if (block_is_plant(block_type))
    {
        float a = BLOCK_SIZE * 0.25f;
        min[0] += a;
        min[2] += a;
        max[0] -= a;
        max[1] -= 2 * a;
        max[2] -= a;
    }

    float tmin = 0.00001f;
    float tmax = 10000.0f;

    vec3 invD;
    for (int i = 0; i < 3; i++)
        invD[i] = 1.0f / front[i];

    vec3 t0s;
    for (int i = 0; i < 3; i++)
        t0s[i] = (min[i] - pos[i]) * invD[i];

    vec3 t1s;
    for (int i = 0; i < 3; i++)
        t1s[i] = (max[i] - pos[i]) * invD[i];

    vec3 tsmaller = {0};
    glm_vec3_minadd(t0s, t1s, tsmaller);

    vec3 tbigger = {0};
    glm_vec3_maxadd(t0s, t1s, tbigger);

    tmin = MAX(tmin, MAX(tsmaller[0], MAX(tsmaller[1], tsmaller[2])));
    tmax = MIN(tmax, MIN(tbigger[0], MIN(tbigger[1], tbigger[2])));

    return (tmin < tmax);
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