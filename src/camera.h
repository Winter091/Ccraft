#include "cglm/cglm.h"
#include "GLFW/glfw3.h"

typedef struct
{
    vec3 pos;
    vec3 front;
    vec3 up;

    float pitch;
    float yaw;

    float fov;
    float sens;
    float move_speed;

    float mouse_last_x;
    float mouse_last_y;

    mat4 view_matrix;
    mat4 proj_matrix;
    mat4 vp_matrix;
}
Camera;

Camera* camera_create(vec3 pos);
void camera_update(Camera* cam, GLFWwindow* window, float dt);