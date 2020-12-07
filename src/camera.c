#include "camera.h"
#include "map.h"

#include "string.h"
#include "stdlib.h"
#include "math.h"

#include "config.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

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
    cam->yaw = -90.0f;

    cam->fov = FOV;
    cam->sens = 0.1f;
    cam->move_speed = 5.0f;

    cam->active = 0;
    cam->mouse_last_x = 0;
    cam->mouse_last_y = 0;

    cam->clip_near = BLOCK_SIZE / 10.0f;

    // at least 512 blocks
    cam->clip_far = MAX((CHUNK_RENDER_RADIUS + 8) * CHUNK_SIZE, 512 * BLOCK_SIZE);

    // generate view, proj and vp matrices
    glm_look(cam->pos, cam->front, cam->up, cam->view_matrix);

    glm_perspective(
        glm_rad(FOV), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 
        cam->clip_near, cam->clip_far, cam->proj_matrix
    );

    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);

    return cam;
}

static void update_mouse_movement(Camera* cam, GLFWwindow* window)
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
    
    vec3 front;
    front[0] = cosf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    front[1] = sinf(glm_rad(cam->pitch));
    front[2] = sinf(glm_rad(cam->yaw)) * cosf(glm_rad(cam->pitch));
    glm_vec3_normalize(front);

    glm_vec3_copy(front, cam->front);
}

static void update_keyboard(Camera* cam, GLFWwindow* window, double dt)
{
    static int key_w, key_s, key_a, key_d, key_shift, key_ctrl;

    key_w     = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    key_s     = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    key_a     = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    key_d     = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    key_shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    key_ctrl  = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

    static vec3 move;
    
    if (key_w || key_s)
    {
        glm_vec3_copy(cam->front, move);
        glm_vec3_scale(move, cam->move_speed * dt, move);
        if (key_s) glm_vec3_scale(move, -1, move);
        glm_vec3_add(cam->pos, move, cam->pos);
    }

    if (key_a || key_d)
    {
        glm_vec3_copy(cam->front, move);

        // can't store move in itself, it will
        // overwrite its own data during computation 
        // and the result won't be correct
        vec3 move2;
        glm_vec3_crossn(move, cam->up, move2);
        
        glm_vec3_scale(move2, cam->move_speed * dt, move2);
        if (key_d) glm_vec3_negate(move2);
        glm_vec3_sub(cam->pos, move2, cam->pos);
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
    int focused = 
        glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
    
    if (!focused)
        cam->active = 0;
    else
    {
        update_mouse_movement(cam, window);
        update_keyboard(cam, window, dt);
    }

    // update view matrix
    glm_look(cam->pos, cam->front, cam->up, cam->view_matrix);

    // then update view-projection matrix
    glm_mat4_mul(cam->proj_matrix, cam->view_matrix, cam->vp_matrix);

    // update frustum planes
    glm_frustum_planes(cam->vp_matrix, cam->frustum_planes);
}

// ray - box hit detection, see
// http://psgraphics.blogspot.com/2016/02/new-simple-ray-box-test-from-andrew.html
int camera_looks_at_block(Camera* cam, int x, int y, int z)
{
    float min[3] = { x * BLOCK_SIZE, y * BLOCK_SIZE, z * BLOCK_SIZE };
    float max[3] = { min[0] + BLOCK_SIZE, min[1] + BLOCK_SIZE, min[2] + BLOCK_SIZE };

    float tmin = 0.001;
    float tmax = 10000;

    for (int a = 0; a < 3; a++) 
    {
        float invD = 1.0f / cam->front[a];
		float t0 = (min[a] - cam->pos[a]) * invD;
		float t1 = (max[a] - cam->pos[a]) * invD;
		if (invD < 0.0f) 
        {
			float temp = t1;
			t1 = t0;
			t0 = temp;
		}

		tmin = t0 > tmin ? t0 : tmin;
		tmax = t1 < tmax ? t1 : tmax;

		if (tmax < tmin)
			return 0;
	}

	return 1;
}

void camera_set_aspect_ratio(Camera* cam, float new_ratio)
{
    glm_perspective_resize(new_ratio, cam->proj_matrix);
    glm_frustum_planes(cam->vp_matrix, cam->frustum_planes);
}