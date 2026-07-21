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


void drawCameraControllerSettings(cameraController& cam)
{
    const bool isOrbitMode = (cam.getNavigationMode() == cameraNavigationMode::Orbit);
    const Color accent = { 90, 160, 255, 255 };

    struct Line { const char* text; bool isHeader; bool isActive; };
    std::vector<Line> lines = {
        { "Current Settings", true, false },
        { TextFormat("Nav Mode: %s", isOrbitMode ? "Orbit" : "Walk"), false, false },
        { TextFormat("Projection: %s", cam.getCameraProjection()), false, false },
        { TextFormat("FOV: %.1f", cam.getFovy()), false, false },
        { TextFormat("Move Speed: %.2f", cam.getWalkSpeed()), false, false },
        { TextFormat("Sensitivity: %.3f", cam.getMouseSensitivity()), false, false },
        { "", false, false },

        { "Orbit Controls", true, isOrbitMode },
        { "Orbit: Drag MMB", false, isOrbitMode },
        { "Pan: Shift + Drag MMB", false, isOrbitMode },
        { "Zoom: Mouse Wheel", false, isOrbitMode },
        { "", false, false },

        { "Walk Controls", true, !isOrbitMode },
        { "Look: Mouse", false, !isOrbitMode },
        { "Move: W A S D", false, !isOrbitMode },
        { "Move Up / Down: Space / Ctrl", false, !isOrbitMode },
        { "Pan: Drag MMB", false, !isOrbitMode },
        { "Rotate (Pitch/Yaw): Arrow Keys", false, !isOrbitMode },
        { "Roll: Q / E", false, !isOrbitMode },
        { "Zoom: Wheel or Numpad +/-", false, !isOrbitMode },
        { "", false, false },

        { "Global Keybinds", true, false },
        { "Toggle Orbit / Walk: N", false, false },
        { "Toggle Projection: Numpad 5", false, false },
        { "Delete Selected: Delete", false, false },
        { "Multi-Select: Ctrl + Click", false, false },
        { "Context Menu: Right Click", false, false },
        { "Toggle Dimensions (Guided): M", false, false },
        { "Toggle This Help: F1", false, false },
    };

    const float paddingX = 10.0f;
    const float paddingY = 8.0f;
    const float headerFontSize = 13.0f;
    const float normalFontSize = 11.0f;
    const float lineHeight = 15.0f;
    const float spacerHeight = 6.0f;

    float maxWidth = 0.0f;
    float contentHeight = 0.0f;
    for (auto& line : lines)
    {
        if (line.text[0] == '\0') { contentHeight += spacerHeight; continue; }
        float fontSize = line.isHeader ? headerFontSize : normalFontSize;
        Vector2 size = MeasureThemeText(line.text, fontSize);
        maxWidth = std::max(maxWidth, size.x);
        contentHeight += lineHeight;
    }

    const int x = 10;
    const int y = 60;
    const int w = static_cast<int>(maxWidth + paddingX * 2.0f);
    const int h = static_cast<int>(contentHeight + paddingY * 2.0f);

    DrawRectangle(x, y, w, h, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    DrawRectangleLines(x, y, w, h, Fade(GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)), 0.5f));

    float ty = static_cast<float>(y) + paddingY;
    for (auto& line : lines)
    {
        if (line.text[0] == '\0') { ty += spacerHeight; continue; }

        float fontSize = line.isHeader ? headerFontSize : normalFontSize;
        Color color = line.isHeader
            ? GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED))
            : (line.isActive ? accent : GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

        DrawThemeText(line.text, static_cast<float>(x) + paddingX, ty, fontSize, color);
        ty += lineHeight;
    }
}