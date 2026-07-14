#include "CameraController.h"
#include "rcamera.h"
#include "ui/themes/themes.h"

float walkSpeed = 20.0f;
float mouseSensitivity = 100.0f;

void InitCamera(Camera3D &camera)
{
    camera.position = { 10.0f, 10.0f, 10.0f };
    camera.target   = { 0.0f, 0.0f, 0.0f };
    camera.up       = { 0.0f, 1.0f, 0.0f };

    // fov is 40f for perspective, keeping similar to blender
    camera.fovy     = 40.0f; // tried updating to blender value...
    camera.projection = CAMERA_PERSPECTIVE;
    
}

void UpdateCameraController(Camera3D &camera)
{   
    float deltaTime = GetFrameTime();

    float moveStep = walkSpeed * deltaTime;

    if (IsKeyDown(KEY_W)) CameraMoveForward(&camera, moveStep, false);
    if (IsKeyDown(KEY_S)) CameraMoveForward(&camera, -moveStep, false);
    if (IsKeyDown(KEY_A)) CameraMoveRight(&camera, -moveStep, false);
    if (IsKeyDown(KEY_D)) CameraMoveRight(&camera, moveStep, false);
    if (IsKeyDown(KEY_Q)) CameraMoveUp(&camera, moveStep);
    if (IsKeyDown(KEY_E)) CameraMoveUp(&camera, -moveStep);
    float wheel = GetMouseWheelMove();

    if (wheel != 0.0f)
    {
        CameraMoveForward(&camera, wheel * 5.0f, false);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mousedis = GetMouseDelta();
        float xangle = mousedis.x * -mouseSensitivity * deltaTime;
        float yangle = mousedis.y * -mouseSensitivity * deltaTime;
        
        UpdateCameraPro(&camera, Vector3{0.0f, 0.0f, 0.0f}, Vector3{xangle, yangle, 0.0f}, 0.0f);
    }
}
// Render a small overlay showing camera controls and current numeric settings
void drawCameraControllerSettings(void)
{
    const int x = 10;
    const int y = 60;
    const int w = 300;
    const int h = 140;

    DrawRectangle(x, y, w, h, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    DrawRectangleLines(x, y, w, h, Fade(GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)), 0.5f));

    int ty = y + 8;
    DrawThemeText("Camera Controls:", (float)x + 8, (float)ty, 12, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED)));
    ty += 20;
    DrawThemeText("Move: W A S D", (float)x + 8, (float)ty, 10, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))); ty += 16;
    DrawThemeText("Up/Down: Q / E", (float)x + 8, (float)ty, 10, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))); ty += 16;
    DrawThemeText("Look: Drag LMB", (float)x + 8, (float)ty, 10, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))); ty += 18;
    DrawThemeText(TextFormat("Walk speed: %.1f", walkSpeed), (float)x + 8, (float)ty, 10, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))); ty += 16;
    DrawThemeText(TextFormat("Mouse sensitivity: %.1f", mouseSensitivity), (float)x + 8, (float)ty, 10, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
}
