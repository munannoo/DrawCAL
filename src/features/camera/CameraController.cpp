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
    float deltaTime = GetFrameTime();
    float walkSpeed = 20.0f; 
    float mouseSensitivity = 100.0f;
    float moveStep = walkSpeed * deltaTime;

    if (IsKeyDown(KEY_W)) CameraMoveForward(&camera, moveStep, false);
    if (IsKeyDown(KEY_S)) CameraMoveForward(&camera, -moveStep, false);
    if (IsKeyDown(KEY_A)) CameraMoveRight(&camera, -moveStep, false);
    if (IsKeyDown(KEY_D)) CameraMoveRight(&camera, moveStep, false);
    if (IsKeyDown(KEY_Q)) CameraMoveUp(&camera, moveStep);
    if (IsKeyDown(KEY_E)) CameraMoveUp(&camera, -moveStep);

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)){
        Vector2 mousedis = GetMouseDelta();
        float xangle = mousedis.x * -mouseSensitivity * deltaTime;
        float yangle = mousedis.y * -mouseSensitivity * deltaTime;
        
        UpdateCameraPro(&camera, Vector3{0.0f, 0.0f, 0.0f}, Vector3{xangle, yangle, 0.0f}, 0.0f);
    }

    if (IsKeyPressed(KEY_Z)){
        camera.target = { 0.0f, 0.0f, 0.0f };
    }
}
