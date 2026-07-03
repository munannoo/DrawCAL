#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H
#include "raylib.h"
// Exposed tunable settings for camera controller (displayed in UI)
#include "raygui.h"
extern float walkSpeed;
extern float mouseSensitivity;

void InitCamera(Camera3D &camera);
void UpdateCameraController(Camera3D &camera);

// Draw an on-screen list of the camera controller settings and keybindings
void drawCameraControllerSettings(void);
#endif