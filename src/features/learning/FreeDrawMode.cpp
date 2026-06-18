#include "FreeDrawMode.h"
#include "raylib.h"
#include "raygui.h"


void freeDrawInit() {
	TraceLog(LOG_INFO, "Initializing Free Draw Mode Scene");
    TraceLog(LOG_INFO, "%d", static_cast<int>(currentScene));
	if (freeDrawState.initiliased) return; // Prevent reinitialization if already initialized
    InitTransformGizmo(); // Initialize the transform gizmo, only needs to be called once
	InitCamera(freeDrawState.camera);
    freeDrawState.drawArea = { 200,140,220,44 };
	freeDrawState.initiliased = true;
	freeDrawState.mouseButtonPressed = false;
	freeDrawState.dropdownEditmode = false;
    freeDrawState.viewIndex = 0;
    freeDrawState.lastViewIndex = 0;
    freeDrawState.viewDropdownOpen = false;
    freeDrawState.cameraLocked = false;
}

void freeDrawUpdate() {

	if (!freeDrawState.initiliased) return; // Prevent update if not initialized
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && freeDrawState.check == 0) {
        DisableCursor();
        freeDrawState.check = 1;
    }
    else if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && freeDrawState.check == 1) {
        ShowCursor();
        freeDrawState.check = 0;
    }
    bool usingGizmo = updateObjectTransformGizmo(freeDrawState.camera);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !usingGizmo) {
        // GetMouseRay identifier not found, so replaced with GetScreenToWorldRay, which does the same thing but takes screen coordinates and camera as input
		Ray ray = GetScreenToWorldRay(GetMousePosition(), freeDrawState.camera);
        //Ray ray = GetMouseRay(GetMousePosition(), freeDrawState.camera);
        leftclick(ray);
    }

    // Allow zoom with mouse wheel when camera is locked (preset view)
    if (freeDrawState.cameraLocked) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            freeDrawState.camera.fovy *= (1.0f - wheel * 0.1f);
            if (freeDrawState.camera.fovy < 5.0f) freeDrawState.camera.fovy = 5.0f;
            if (freeDrawState.camera.fovy > 120.0f) freeDrawState.camera.fovy = 120.0f;
        }
    }
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
        ShowCursor();
    }

    //if (currentResIndex != lastResIndex) {
    //    SetWindowSize(cr[currentResIndex].width, cr[currentResIndex].height);
    //    lastResIndex = currentResIndex;
    //}

    if (!usingGizmo) {
        if (!freeDrawState.cameraLocked) UpdateCameraController(freeDrawState.camera);
    }




}

void freeDrawDraw() {
    DrawCameraScene(freeDrawState.camera);
    topBar(currentResIndex, freeDrawState.dropdownEditmode);
    // Top-right view dropdown
    const int viewW = 140;
    const int viewH = 30;
    Rectangle viewRect = { (float)GetScreenWidth() - viewW - 10.0f, 5.0f, (float)viewW, (float)viewH };
    const char* viewOptions = "Free;Front;Top;Left;Right";
    // When GuiDropdownBox returns true it toggles the open state
    if (GuiDropdownBox(viewRect, viewOptions, &freeDrawState.viewIndex, freeDrawState.viewDropdownOpen)) {
        freeDrawState.viewDropdownOpen = !freeDrawState.viewDropdownOpen;
    }

    // If view selection changed, apply camera preset and lock camera movement
    if (freeDrawState.viewIndex != freeDrawState.lastViewIndex) {
        // Apply presets based on selection (0 = Free/unlocked)
        switch (freeDrawState.viewIndex) {
            case 0: // Free - restore default controller camera
                InitCamera(freeDrawState.camera);
                freeDrawState.cameraLocked = false;
                break;
            case 1: // Front
                freeDrawState.camera.position = { 0.0f, 0.0f, 10.0f };
                freeDrawState.camera.target = { 0.0f, 0.0f, 0.0f };
                freeDrawState.camera.up = { 0.0f, 1.0f, 0.0f };
                freeDrawState.camera.projection = CAMERA_PERSPECTIVE;
                freeDrawState.camera.fovy = 45.0f;
                freeDrawState.cameraLocked = true;
                break;
            case 2: // Top
                freeDrawState.camera.position = { 0.0f, 10.0f, 0.0f };
                freeDrawState.camera.target = { 0.0f, 0.0f, 0.0f };
                freeDrawState.camera.up = { 0.0f, 0.0f, -1.0f };
                freeDrawState.camera.projection = CAMERA_ORTHOGRAPHIC;
                freeDrawState.camera.fovy = 45.0f;
                freeDrawState.cameraLocked = true;
                break;
            case 3: // Left
                freeDrawState.camera.position = { -10.0f, 0.0f, 0.0f };
                freeDrawState.camera.target = { 0.0f, 0.0f, 0.0f };
                freeDrawState.camera.up = { 0.0f, 1.0f, 0.0f };
                freeDrawState.camera.projection = CAMERA_PERSPECTIVE;
                freeDrawState.camera.fovy = 45.0f;
                freeDrawState.cameraLocked = true;
                break;
            case 4: // Right
                freeDrawState.camera.position = { 10.0f, 0.0f, 0.0f };
                freeDrawState.camera.target = { 0.0f, 0.0f, 0.0f };
                freeDrawState.camera.up = { 0.0f, 1.0f, 0.0f };
                freeDrawState.camera.projection = CAMERA_PERSPECTIVE;
                freeDrawState.camera.fovy = 45.0f;
                freeDrawState.cameraLocked = true;
                break;
        }
        freeDrawState.lastViewIndex = freeDrawState.viewIndex;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || freeDrawState.mouseButtonPressed) {
        contextMenu(freeDrawState.mouseButtonPressed, freeDrawState.camera); // under InputHandler.cpp
    }
}

void freeDrawUnload() {
    freeDrawState.initiliased = false;
    UnloadTransformGizmo();

	// Put texture unload function here
}