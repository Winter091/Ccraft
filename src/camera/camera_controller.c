#include <camera/camera_controller.h>

#include <assert.h>

#include <string.h>
#include <time_measure.h>
#include <config.h>
#include <window.h>

CameraController* cameracontroller_create(Camera* camera)
{
    CameraController* cc = malloc(sizeof(CameraController));

    assert(camera);
    cc->camera = camera;
    cc->is_controlling = 1;

    memset(&cc->tracked_object_info, 0, sizeof(ObjectLocationInfo));
    cc->is_tracking = 0;

    cc->pos_delegate = NULL;
    cc->rot_delegate = NULL;

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

void cc_set_pos_update_delegate(CameraController* cc, camera_position_delegate delegate)
{
    assert(delegate);
    cc->pos_delegate = delegate;
}

void cc_set_rot_update_delegate(CameraController* cc, camera_rotation_delegate delegate)
{
    assert(delegate);
    cc->rot_delegate = delegate;
}

static void do_kb_input(CameraController* cc)
{
    Camera* cam = cc->camera;

    int const key_c        = window_is_key_pressed(GLFW_KEY_C);
    int const key_pageup   = window_is_key_pressed(GLFW_KEY_PAGE_UP);
    int const key_pagedown = window_is_key_pressed(GLFW_KEY_PAGE_DOWN);
    int const key_tab      = window_is_key_pressed(GLFW_KEY_TAB);
    static int tab_already_pressed = 0;

    float dt = dt_get();

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

    /*
    // Handle fly/walk mode
    if (key_tab && !tab_already_pressed)
    {
        if (cam->is_fly_mode == 1)
            cam->is_fly_mode = 0;
        else
            cam->is_fly_mode = 1;
        
        tab_already_pressed = 1;
    }
    */
}

void cc_do_control(CameraController* cc)
{
    assert(cc->pos_delegate);
    assert(cc->rot_delegate);

    glm_vec3_copy(cc->camera->pos, cc->camera->prev_pos);
    do_kb_input(cc);

    cc->pos_delegate(cc);
    cc->rot_delegate(cc);

    camera_update_matrices(cc->camera);
}

void cc_first_person_pos_update(void* cc)
{
    CameraController* controller = (CameraController*)cc;

    glm_vec3_copy(controller->tracked_object_info.pos, controller->camera->pos);
}

void cc_first_person_rot_update(void* cc)
{
    CameraController* controller = (CameraController*)cc;

    glm_vec3_copy(controller->tracked_object_info.front, controller->camera->front);
    controller->camera->pitch = *controller->tracked_object_info.pitch;
    controller->camera->yaw = *controller->tracked_object_info.yaw;
}

void cameracontroller_destroy(CameraController* cc)
{
    free(cc);
}
