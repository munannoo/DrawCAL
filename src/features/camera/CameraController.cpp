#include "CameraController.h"
#include "rcamera.h"
void InitCamera(Camera3D &camera)
{
    camera.position = { 10.0f, 10.0f, 10.0f };
    camera.target   = { 0.0f, 0.0f, 0.0f };
    camera.up       = { 0.0f, 1.0f, 0.0f };
    camera.fovy     = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
}

void UpdateCameraController(Camera3D &camera)
{
    float walkSpeed = 0.1f;
    if (IsKeyDown(KEY_W)) CameraMoveForward(&camera, walkSpeed, false);
    if (IsKeyDown(KEY_S)) CameraMoveForward(&camera, -walkSpeed, false);
    if (IsKeyDown(KEY_A)) CameraMoveRight(&camera, -walkSpeed, false);
    if (IsKeyDown(KEY_D)) CameraMoveRight(&camera, walkSpeed, false);
    if (IsKeyDown(KEY_Q)) CameraMoveUp(&camera, walkSpeed);
    if (IsKeyDown(KEY_E)) CameraMoveUp(&camera, -walkSpeed);

    if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
        Vector2 mousedis = GetMouseDelta();
        float xangle = mousedis.x * -0.03f;
        float yangle = mousedis.y * -0.03f;
        UpdateCameraPro(&camera, Vector3{0.0f, 0.0f, 0.0f}, Vector3{xangle, yangle, 0.0f}, 0.0f);
    }

    if (IsKeyPressed(KEY_Z)){
        camera.target = { 0.0f, 0.0f, 0.0f };
    }

}

