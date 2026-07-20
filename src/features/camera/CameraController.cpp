#include "CameraController.h"

editorViewMode currentViewMode = editorViewMode::Single;

cameraController::cameraController(Vector3 position, Vector3 target, Vector3 up, CameraProjection projection)
{
    camera.position = position;
    camera.target = target;
    camera.up = up;
    camera.projection = projection;
    setCameraPerspective();
}

void cameraController::setView(cameraView view, Vector3 target)
{
    switch (view)
    {
    case cameraView::Free:
        camera.position = { 10.0f, 10.0f, 10.0f };
        camera.target = target;
        camera.up = { 0.0f, 1.0f, 0.0f };
        break;

    case cameraView::Top:
        camera.position = { target.x, 20.0f, target.z };
        camera.target = target;
        camera.up = { 0.0f, 0.0f, -1.0f };
        break;

    case cameraView::Front:
        camera.position = { target.x, target.y, target.z + 20.0f };
        camera.target = target;
        camera.up = { 0.0f, 1.0f, 0.0f };
        break;

    case cameraView::Left:
        camera.position = { target.x - 20.0f, target.y, target.z };
        camera.target = target;
        camera.up = { 0.0f, 1.0f, 0.0f };
        break;

    case cameraView::Right:
        camera.position = { target.x + 20.0f, target.y, target.z };
        camera.target = target;
        camera.up = { 0.0f, 1.0f, 0.0f };
        break;
    }

    currentView = view;
}

void cameraController::updateCamera() {
    if (navigationMode == cameraNavigationMode::Walk)
    {
        // Camera mode: CAMERA_FREE, CAMERA_FIRST_PERSON, CAMERA_THIRD_PERSON, CAMERA_ORBITAL or CUSTOM
        // Custon makes the function do nothing

        // CAPITAL U UPDATE CAMERA IS A RAYLIB FUNCTION, NOT A CLASS METHOD, small u updateCamera is a class method
        UpdateCamera(&camera, CAMERA_FREE);
    }
    else
    {
        updateBlender();
    }
}

void cameraController::updateBlender()
{
    Vector2 mouseDelta = GetMouseDelta();

    Vector3 offset = Vector3Subtract(camera.position, camera.target);
    float distance = Vector3Length(offset);

    // Orbit (MMB)
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && !IsKeyDown(KEY_LEFT_SHIFT)) {
        float yaw = -mouseDelta.x * orbitSensitivity * GetFrameTime();

        float pitch = -mouseDelta.y * orbitSensitivity * GetFrameTime();

        Matrix yawMatrix = MatrixRotateY(yaw);

        Vector3 right = Vector3Normalize(Vector3CrossProduct(camera.up, offset));

        Matrix pitchMatrix = MatrixRotate(right, pitch);

        offset = Vector3Transform(offset, yawMatrix);
        offset = Vector3Transform(offset, pitchMatrix);

        camera.position = Vector3Add(camera.target, offset);
    }

    // Panning (Shift + MMB)
    else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && IsKeyDown(KEY_LEFT_SHIFT))
    {
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

        Vector3 move = Vector3Add(Vector3Scale(right, -mouseDelta.x * panSensitivity), Vector3Scale(camera.up, mouseDelta.y * panSensitivity));

        camera.position = Vector3Add(camera.position, move);

        camera.target = Vector3Add(camera.target, move);
    }

    // Ctrl + MMB drag (Blender-style dolly)
    else if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
        float zoomDelta = mouseDelta.y * zoomSensitivity * GetFrameTime();

        if (camera.projection == CAMERA_PERSPECTIVE) {
            distance += zoomDelta;

            if (distance < 0.1f) distance = 0.1f;

            offset = Vector3Scale(Vector3Normalize(offset), distance);
            camera.position = Vector3Add(camera.target, offset);
        }
        else {
            orthoScale += zoomDelta;

            if (orthoScale < 0.1f) orthoScale = 0.1f;

            camera.fovy = orthoScale;
        }
    }

    // Zooming (Ctrl + MMB or Scroll Wheel)
    float wheel = GetMouseWheelMove();

    if (wheel != 0.0f && !IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        if (camera.projection == CAMERA_PERSPECTIVE) {
            distance -= wheel * zoomSensitivity;

            if (distance < 0.1f) distance = 0.1f;

            offset = Vector3Scale(Vector3Normalize(offset), distance);
            camera.position = Vector3Add(camera.target, offset);
        }
        else {
            orthoScale -= wheel * zoomSensitivity;

            if (orthoScale < 0.1f) orthoScale = 0.1f;

            camera.fovy = orthoScale;
        }
    }
}

void cameraController::toggleProjection()
{
    if (camera.projection == CAMERA_PERSPECTIVE)
        setCameraOrthographic();
    else
        setCameraPerspective();
}

void cameraController::setCameraOrthographic()
{
    camera.fovy = orthoScale; // near plane height in world units for orthographic projection
    camera.projection = CAMERA_ORTHOGRAPHIC;
}

void cameraController::setCameraPerspective()
{
    // Blender's typical FOV equivalent to ~50mm lens is close to 39-40 degrees
    camera.fovy = perspectiveFov;
    camera.projection = CAMERA_PERSPECTIVE;
}

// Render a small overlay showing camera controls and current numeric settings
// Render a small overlay showing camera controls and current numeric settings
void drawCameraControllerSettings(void)
{
    const int x = 10;
    const int y = 60;
    const int w = 350;
    const int h = 200;

    DrawRectangle(x, y, w, h, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    DrawRectangleLines(x, y, w, h, Fade(GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)), 0.5f));

    int ty = y + 8;



    DrawThemeText("R3D Camera (Orbit Mode):", (float)x + 8, (float)ty, 12, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED)));
    ty += 20;
    DrawThemeText("Orbit: Drag MMB", (float)x + 8, (float)ty, 10, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))); ty += 14;
    DrawThemeText("Pan: Shift + Drag MMB", (float)x + 8, (float)ty, 10, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))); ty += 14;
    DrawThemeText("Zoom: Ctrl+MMB or Wheel", (float)x + 8, (float)ty, 10, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))); ty += 14;

    ty += 2;

}
