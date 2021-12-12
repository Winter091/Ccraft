#ifndef CAMERA_CONTROLLER_H_
#define CAMERA_CONTROLLER_H_

#include <cglm/cglm.h>

#include <camera/camera.h>

typedef struct 
{
    float* pos;
    float* front;
    float* up;

    float* pitch;
    float* yaw;
}
ObjectLocationInfo;

typedef void (*camera_update_func)(void* cc);

typedef struct
{
    Camera* camera;
    int is_controlling;
    float fly_speed;

    ObjectLocationInfo tracked_object_info;
    int is_tracking;

    camera_update_func update_func;
} 
CameraController;


CameraController* cameracontroller_create(Camera* camera);

void cameracontroller_set_camera(CameraController* cc, Camera* camera);

void cameracontroller_set_track_object(CameraController* cc, ObjectLocationInfo* object_info);

void cameracontroller_set_tracking(CameraController* cc, int is_tracking);

void cameracontroller_set_update_func(CameraController* cc, camera_update_func func);

void cameracontroller_do_control(CameraController* cc);

void cameracontroller_first_person_update(void* cc);

void cameracontroller_destroy(CameraController* cc);

#endif