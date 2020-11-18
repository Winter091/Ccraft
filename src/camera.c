#include "camera.h"

#include "string.h"
#include "stdlib.h"
#include "math.h"

#include "config.h"

Camera* camera_create(vec3 pos)
{
    Camera* cam = malloc(sizeof(Camera));

    memcpy(cam->pos, pos, sizeof(vec3));

    cam->front[0] = 0.0f;
    cam->front[1] = 0.0f;
    cam->front[2] = -1.0f;
    
    cam->up[0] = 0.0f;
    cam->up[1] = 1.0f;
    cam->up[2] = 0.0f;

    cam->pitch = 0.0f;
    cam->yaw = 90.0f;

    cam->fov = FOV;
    cam->sens = 0.1f;
    cam->move_speed = 0.0015f;

    cam->mouse_last_x = 0;
    cam->mouse_last_y = 0;

    vec3 temp;
    glm_vec3_add(cam->pos, cam->front, temp);
    glm_lookat(cam->pos, temp, cam->up, cam->view_matrix);

    glm_perspective(
        glm_rad(FOV), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 
        0.01f, 500.0f, cam->proj_matrix
    );

    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);

    return cam;
}

static void update_mouse(Camera* cam, GLFWwindow* window)
{ 
    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);

    static int first_update = 1;
    if (first_update)
    {
        first_update = 0;
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
    
    vec3 front;
    front[0] = cosf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    front[1] = sinf(glm_rad(cam->pitch));
    front[2] = sinf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    glm_normalize(front);

    memcpy(cam->front, front, sizeof(vec3));
}

static void update_keyboard(Camera* cam, GLFWwindow* window, float dt)
{
    vec3 move;
    memcpy(move, cam->front, sizeof(vec3));
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        glm_vec3_scale(move, cam->move_speed * dt, move);
        glm_vec3_add(cam->pos, move, cam->pos);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        glm_vec3_scale(move, cam->move_speed * dt, move);
        glm_vec3_sub(cam->pos, move, cam->pos);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        vec3 res;
        glm_vec3_crossn(move, cam->up, res);
        glm_vec3_scale(res, cam->move_speed * dt, res);
        glm_vec3_sub(cam->pos, res, cam->pos);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        vec3 res;
        glm_vec3_crossn(move, cam->up, res);
        glm_vec3_scale(res, cam->move_speed * dt, res);
        glm_vec3_add(cam->pos, res, cam->pos);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        memcpy(move, cam->up, sizeof(vec3));
        glm_vec3_scale(move, cam->move_speed * dt, move);
        glm_vec3_add(cam->pos, move, cam->pos);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        memcpy(move, cam->up, sizeof(vec3));
        glm_vec3_scale(move, cam->move_speed * dt, move);
        glm_vec3_sub(cam->pos, move, cam->pos);
    }

}

void camera_update(Camera* cam, GLFWwindow* window, float dt)
{
    update_mouse(cam, window);
    update_keyboard(cam, window, dt);

    // update view matrix
    vec3 temp;
    glm_vec3_add(cam->pos, cam->front, temp);
    glm_lookat(cam->pos, temp, cam->up, cam->view_matrix);

    // then update view-projection matrix
    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);
}