#include "camera.h"
#include "stdlib.h"
#include "math.h"
#include "config.h"
#include "utils.h"
#include "block.h"

Camera* camera_create()
{
    Camera* cam = malloc(sizeof(Camera));

    cam->active = 1;
    cam->fly_mode = 0;

    cam->pos[0] = 0.0f;
    cam->pos[1] = CHUNK_HEIGHT * BLOCK_SIZE;
    cam->pos[2] = 0.0f;
    glm_vec3_copy(cam->pos, cam->prev_pos);

    glm_vec3_fill(cam->frame_motion, 0.0f);
    glm_vec2_fill(cam->motion_horizontal, 0.0f);
    cam->motion_vertical = 0.0f;
    
    cam->front[0] = 0.0f;
    cam->front[1] = 0.0f;
    cam->front[2] = -1.0f;

    cam->up[0] = 0.0f;
    cam->up[1] = 1.0f;
    cam->up[2] = 0.0f;

    cam->pitch = 0.0f;
    cam->yaw = 270.0f;

    cam->fov = FOV;
    cam->sens = 0.1f;

    cam->first_frame = 1;
    cam->mouse_last_x = 0;
    cam->mouse_last_y = 0;

    cam->aspect_ratio = (float)WINDOW_WIDTH / WINDOW_HEIGHT;

    cam->clip_near = BLOCK_SIZE / 10.0f;
    // at least 512 blocks
    cam->clip_far = MAX((CHUNK_RENDER_RADIUS * 1.2f) * CHUNK_SIZE, 512 * BLOCK_SIZE);

    // generate view, proj and vp matrices
    glm_look(cam->pos, cam->front, cam->up, cam->view_matrix);
    glm_perspective(
        glm_rad(FOV), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 
        cam->clip_near, cam->clip_far, cam->proj_matrix
    );
    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);
    glm_mat4_copy(cam->view_matrix, cam->prev_view_matrix);

    return cam;
}

void camera_update_view_dir(Camera* cam, GLFWwindow* window)
{
    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);

    if (cam->first_frame)
    {
        cam->first_frame = 0;
        cam->mouse_last_x = mouse_x;
        cam->mouse_last_y = mouse_y;
        return;
    }

    double dx = mouse_x - cam->mouse_last_x;
    double dy = mouse_y - cam->mouse_last_y;

    cam->mouse_last_x = mouse_x;
    cam->mouse_last_y = mouse_y;

    cam->yaw += dx * cam->sens;
    cam->pitch -= dy * cam->sens;

    if (cam->pitch < -89.9f)
        cam->pitch = -89.9f;
    else if (cam->pitch > 89.9f)
        cam->pitch = 89.9f;

    if (cam->yaw >= 360.0f) cam->yaw -= 360.0f;
    if (cam->yaw <= 0.0f) cam->yaw += 360.0f;

    cam->front[0] = cosf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    cam->front[1] = sinf(glm_rad(cam->pitch));
    cam->front[2] = sinf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    glm_vec3_normalize(cam->front);
}

void camera_update_parameters(Camera* cam, GLFWwindow* window, double dt)
{
    glm_vec3_copy(cam->pos, cam->prev_pos);

    if (!cam->active)
        return;

    int key_c   = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    int key_tab = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
    static int tab_already_pressed = 0;

    // Handle zoom mode
    if (key_c && cam->fov == FOV)
        camera_set_fov(cam, FOV_ZOOM);
    else if (!key_c && cam->fov == FOV_ZOOM)
        camera_set_fov(cam, FOV);
    
    // Handle fly/walk mode

    // Little hack to prevent changing move every frame
    // when tab is pressed
    if (!key_tab && tab_already_pressed)
        tab_already_pressed = 0;

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