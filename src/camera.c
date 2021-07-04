#include "camera.h"

#include "stdlib.h"
#include "math.h"

#include "config.h"
#include "utils.h"
#include "block.h"
#include "window.h"

Camera* camera_create()
{
    Camera* cam = malloc(sizeof(Camera));

    cam->active = 1;
    cam->fly_mode = 0;

    // Position will be set later, using database or
    // moving player to ground level
    glm_vec3_fill(cam->pos, 0.0f);
    glm_vec3_fill(cam->prev_pos, 0.0f);

    glm_vec3_fill(cam->frame_motion, 0.0f);
    glm_vec2_fill(cam->speed_horizontal, 0.0f);
    cam->speed_vertical = 0.0f;

    my_glm_vec3_set(cam->front, 0.0f, 0.0f, -1.0f);
    my_glm_vec3_set(cam->up,    0.0f, 1.0f,  0.0f);

    cam->pitch = 0.0;
    cam->yaw = 270.0;

    cam->fov = FOV;
    cam->sens = MOUSE_SENS;
    cam->fly_speed = 20.0f * BLOCK_SIZE;

    cam->first_frame = 1;
    cam->mouse_last_x = 0;
    cam->mouse_last_y = 0;

    cam->aspect_ratio = (float)WINDOW_WIDTH / WINDOW_HEIGHT;

    cam->clip_near = BLOCK_SIZE / 10.0f;
    // At least 512 blocks
    cam->clip_far = MAX((CHUNK_RENDER_RADIUS * 1.2f) * CHUNK_SIZE, 512 * BLOCK_SIZE);

    // Generate matrices
    glm_look(cam->pos, cam->front, cam->up, cam->view_matrix);
    glm_perspective(
        glm_rad(cam->fov), (float)WINDOW_WIDTH / WINDOW_HEIGHT,
        cam->clip_near, cam->clip_far, cam->proj_matrix
    );
    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);
    glm_mat4_copy(cam->view_matrix, cam->prev_view_matrix);
    
    return cam;
}

void camera_update_view_dir(Camera* cam)
{
    double mouse_x, mouse_y;
    glfwGetCursorPos(g_window->glfw, &mouse_x, &mouse_y);

    if (cam->first_frame)
    {
        cam->first_frame = 0;
        cam->mouse_last_x = mouse_x;
        cam->mouse_last_y = mouse_y;
        return;
    }

    float dx = mouse_x - cam->mouse_last_x;
    float dy = mouse_y - cam->mouse_last_y;

    cam->mouse_last_x = mouse_x;
    cam->mouse_last_y = mouse_y;

    cam->yaw += dx * cam->sens;
    cam->pitch -= dy * cam->sens;

    if (cam->pitch < -89.5f)
        cam->pitch = -89.5f;
    else if (cam->pitch > 89.5f)
        cam->pitch = 89.5f;

    if (cam->yaw > 360.0f) cam->yaw -= 360.0f;
    if (cam->yaw < 0.0f) cam->yaw += 360.0f;

    cam->front[0] = cosf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    cam->front[1] = sinf(glm_rad(cam->pitch));
    cam->front[2] = sinf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    glm_vec3_normalize(cam->front);
}

void camera_update_parameters(Camera* cam, float dt)
{
    glm_vec3_copy(cam->pos, cam->prev_pos);

    if (!cam->active)
        return;

    int key_c        = (glfwGetKey(g_window->glfw, GLFW_KEY_C)         == GLFW_PRESS);
    int key_pageup   = (glfwGetKey(g_window->glfw, GLFW_KEY_PAGE_UP)   == GLFW_PRESS);
    int key_pagedown = (glfwGetKey(g_window->glfw, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);
    int key_tab      = (glfwGetKey(g_window->glfw, GLFW_KEY_TAB)       == GLFW_PRESS);
    static int tab_already_pressed = 0;

    // Handle fly speed
    if (key_pageup)
        cam->fly_speed *= (1.0f + dt);
    else if (key_pagedown)
        cam->fly_speed /= (1.0f + dt);

    // Handle zoom mode
    if (key_c && cam->fov == FOV)
        camera_set_fov(cam, FOV_ZOOM);
    else if (!key_c && cam->fov == FOV_ZOOM)
        camera_set_fov(cam, FOV);
    
    // Little hack to prevent changing fly/walk move
    // every frame when tab is pressed
    if (!key_tab && tab_already_pressed)
        tab_already_pressed = 0;

    // Handle fly/walk mode
    if (key_tab && !tab_already_pressed)
    {
        if (cam->fly_mode == 1)
            cam->fly_mode = 0;
        else
            cam->fly_mode = 1;
        
        tab_already_pressed = 1;
    }
}

void camera_update_matrices(Camera* cam)
{
    // update vp matrices
    glm_mat4_copy(cam->view_matrix, cam->prev_view_matrix);
    glm_look(cam->pos, cam->front, cam->up, cam->view_matrix);
    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);

    // update frustum planes
    glm_frustum_planes(cam->vp_matrix, cam->frustum_planes);
}

// ray - aabb hit detection, see
// https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
int camera_looks_at_block(Camera* cam, int x, int y, int z, unsigned char block_type)
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
        invD[i] = 1.0f / cam->front[i];

    vec3 t0s;
    for (int i = 0; i < 3; i++)
        t0s[i] = (min[i] - cam->pos[i]) * invD[i];

    vec3 t1s;
    for (int i = 0; i < 3; i++)
        t1s[i] = (max[i] - cam->pos[i]) * invD[i];

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