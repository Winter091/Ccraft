#include <camera/camera_controller.h>

#include <assert.h>

#include <string.h>
#include <time_measure.h>
#include <config.h>
#include <window.h>

static void on_keyboard_key(void* this_object, int glfw_keycode, int glfw_action_code)
{
    CameraController* cc = (CameraController*)this_object;

    if (glfw_keycode == GLFW_KEY_C)
    {
        if (glfw_action_code == GLFW_PRESS && cc->camera->fov == FOV)
            camera_set_fov(cc->camera, FOV_ZOOM);
        else if (glfw_action_code == GLFW_RELEASE && cc->camera->fov == FOV_ZOOM)
            camera_set_fov(cc->camera, FOV);
    }
}

CameraController* cameracontroller_create(Camera* camera)
{
    CameraController* cc = malloc(sizeof(CameraController));

    assert(camera);
    cc->camera = camera;
    cc->is_controlling = 1;
    cc->fly_speed = 20.0f * BLOCK_SIZE;

    memset(&cc->tracked_object_info, 0, sizeof(ObjectLocationInfo));
    cc->is_tracking = 0;

    cc->update_func = NULL;

    register_keyboard_key_press_callback(cc, on_keyboard_key);

    return cc;
}

void cameracontroller_set_camera(CameraController* cc, Camera* camera)
{
    assert(camera);
    cc->camera = camera;
}

void cameracontroller_set_track_object(CameraController* cc, ObjectLocationInfo* object_info)
{
    if (object_info)
        memcpy(&cc->tracked_object_info, object_info, sizeof(ObjectLocationInfo));
}

void cameracontroller_set_tracking(CameraController* cc, int is_tracking)
{
    assert(is_tracking && cc->tracked_object_info.front), "What am I gonna track?";
    cc->is_tracking = is_tracking;
}

void cameracontroller_set_update_func(CameraController* cc, camera_update_func func)
{
    assert(func);
    cc->update_func = func;
}

static void update_fly_speed(CameraController* cc)
{
    int const key_pageup   = window_is_key_pressed(GLFW_KEY_PAGE_UP);
    int const key_pagedown = window_is_key_pressed(GLFW_KEY_PAGE_DOWN);
    float const dt = dt_get();

    if (key_pageup)
        cc->fly_speed *= (1.0f + dt);
    else if (key_pagedown)
        cc->fly_speed /= (1.0f + dt);
}

void cameracontroller_do_control(CameraController* cc)
{
    glm_vec3_copy(cc->camera->pos, cc->camera->prev_pos);
    update_fly_speed(cc);

    assert(cc->update_func);
    cc->update_func(cc);

    camera_update_matrices(cc->camera);
}

void cameracontroller_first_person_update(void* cc)
{
    CameraController* controller = (CameraController*)cc;

    // Pos
    glm_vec3_copy(controller->tracked_object_info.pos, controller->camera->pos);

    // Rotation
    glm_vec3_copy(controller->tracked_object_info.front, controller->camera->front);
    controller->camera->pitch = *controller->tracked_object_info.pitch;
    controller->camera->yaw = *controller->tracked_object_info.yaw;
}

void cameracontroller_destroy(CameraController* cc)
{
    free(cc);
}
