#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "raylib.h"

// Initialize the camera structure
void InitCamera(Camera3D &camera);

// Update camera logic (call every frame)
void UpdateCameraController(Camera3D &camera);



#endif // CAMERA_CONTROLLER_H