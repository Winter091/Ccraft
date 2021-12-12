#ifndef CAMERA_CONTROLLER_H_
#define CAMERA_CONTROLLER_H_

#include "cglm/cglm.h"
#include "camera.h"

typedef struct 
{
    float* pos;
    float* front;
    float* up;

    float* pitch;
    float* yaw;
}
ObjectLocationInfo;

typedef void (*camera_position_delegate)(void* cc);
typedef void (*camera_rotation_delegate)(void* cc);

typedef struct
{
    Camera* camera;
    int is_controlling;

    ObjectLocationInfo tracked_object_info;
    int is_tracking;

    camera_position_delegate pos_delegate;
    camera_rotation_delegate rot_delegate;
} 
CameraController;


CameraController* cameracontroller_create(Camera* camera);

void cameracontroller_set_camera(CameraController* cc, Camera* camera);

void cameracontroller_set_track_object(CameraController* cc, ObjectLocationInfo* object_info);

void cameracontroller_set_tracking(CameraController* cc, int is_tracking);

void cc_set_pos_delegate(CameraController* cc, camera_position_delegate delegate);

void cc_set_rot_delegate(CameraController* cc, camera_rotation_delegate delegate);

void cc_do_control(CameraController* cc);

void cc_1p_pos_delegate(void* cc);

void cc_1p_rot_delegate(void* cc);

void cameracontroller_destroy(CameraController* cc);

#endif