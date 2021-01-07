#include "camera.h"
#include "stdlib.h"
#include "math.h"
#include "config.h"
#include "utils.h"

Camera* camera_create(vec3 pos, vec3 dir)
{
    Camera* cam = malloc(sizeof(Camera));

    glm_vec3_copy(pos, cam->pos);
    glm_vec3_copy(pos, cam->prev_pos);
    glm_vec3_copy(dir, cam->front);

    cam->up[0] = 0.0f;
    cam->up[1] = 1.0f;
    cam->up[2] = 0.0f;

    cam->pitch = 0.0f;
    cam->yaw = -90.0f;

    cam->fov = FOV;
    cam->sens = 0.1f;
    cam->move_speed = 15.0f * BLOCK_SIZE;

    cam->active = 0;
    cam->mouse_last_x = 0;
    cam->mouse_last_y = 0;

    cam->aspect_ratio = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    cam->clip_near = BLOCK_SIZE / 10.0f;

    // at least 512 blocks
    cam->clip_far = MAX((CHUNK_RENDER_RADIUS * 1.1f) * CHUNK_SIZE, 512 * BLOCK_SIZE);

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

static void update_mouse(Camera* cam, GLFWwindow* window)
{ 
    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);

    if (!cam->active)
    {
        cam->active = 1;
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

static void update_keyboard(Camera* cam, GLFWwindow* window, double dt)
{
    static int key_w, key_s, key_a, key_d, key_shift, 
        key_ctrl, key_c, key_pageup, key_pagedown;

    key_w        = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    key_s        = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    key_a        = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    key_d        = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    key_shift    = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    key_ctrl     = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
    key_c        = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    key_pageup   = glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS;
    key_pagedown = glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS;

    // handle zoom mode
    if (key_c && cam->fov == FOV)
        camera_set_fov(cam, FOV_ZOOM);
    else if (!key_c && cam->fov == FOV_ZOOM)
        camera_set_fov(cam, FOV);

    // handle move speed
    if (key_pageup)
        cam->move_speed *= 1.003f;
    if (key_pagedown)
        cam->move_speed /= 1.003f;

    vec3 move;

    if (key_w || key_s)
    {
        glm_vec3_copy(cam->front, move);

        glm_vec3_scale(move, cam->move_speed * dt, move);
        if (key_s) glm_vec3_negate(move);
        glm_vec3_add(cam->pos, move, cam->pos);
    }

    if (key_a || key_d)
    {
        glm_vec3_copy(cam->front, move);

        vec3 move_side;
        glm_vec3_crossn(move, cam->up, move_side);
        
        glm_vec3_scale(move_side, cam->move_speed * dt, move_side);
        if (key_d) glm_vec3_negate(move_side);
        glm_vec3_sub(cam->pos, move_side, cam->pos);
    }

    if (key_shift || key_ctrl)
    {
        glm_vec3_copy(cam->up, move);

        glm_vec3_scale(move, cam->move_speed * dt, move);
        if (key_ctrl) glm_vec3_negate(move);
        glm_vec3_add(cam->pos, move, cam->pos);
    }
}

void camera_update(Camera* cam, GLFWwindow* window, double dt)
{
    int focused = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
    
    if (!focused)
        cam->active = 0;
    else
    {
        glm_vec3_copy(cam->pos, cam->prev_pos);
        update_mouse(cam, window);
        update_keyboard(cam, window, dt);
    }

    // update vp matrices
    glm_mat4_copy(cam->view_matrix, cam->prev_view_matrix);
    glm_look(cam->pos, cam->front, cam->up, cam->view_matrix);
    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);

    // update frustum planes
    glm_frustum_planes(cam->vp_matrix, cam->frustum_planes);
}

// ray - axis aligned box hit detection, see
// https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
int camera_looks_at_block(Camera* cam, int x, int y, int z)
{
    vec3 min = { x * BLOCK_SIZE, y * BLOCK_SIZE, z * BLOCK_SIZE };
    vec3 max = { min[0] + BLOCK_SIZE, min[1] + BLOCK_SIZE, min[2] + BLOCK_SIZE };

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