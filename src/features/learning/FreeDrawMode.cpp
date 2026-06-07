#include "FreeDrawMode.h"

void freeDrawInit() {
    TraceLog(LOG_INFO, "freeDrawInit called");
	if (freeDrawState.initiliased) return; // Prevent reinitialization if already initialized
    InitTransformGizmo(); // Initialize the transform gizmo, only needs to be called once
	InitCamera(freeDrawState.camera);
    freeDrawState.drawArea = { 200,140,220,44 };
	freeDrawState.initiliased = true;
	freeDrawState.mouseButtonPressed = false;
	freeDrawState.dropdownEditmode = false;
}

void freeDrawUpdate() {
    TraceLog(LOG_INFO, "freeDrawUpdate called");

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
        Ray ray = GetMouseRay(GetMousePosition(), freeDrawState.camera);
        leftclick(ray);
    }
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
        ShowCursor();
    }

    if (currentResIndex != lastResIndex) {
        SetWindowSize(cr[currentResIndex].width, cr[currentResIndex].height);
        lastResIndex = currentResIndex;
    }

    if (!usingGizmo) {
        UpdateCameraController(freeDrawState.camera);
    }




}

void freeDrawDraw() {
    DrawCameraScene(freeDrawState.camera);
    topBar(currentResIndex, freeDrawState.dropdownEditmode);
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || freeDrawState.mouseButtonPressed) {
        contextMenu(freeDrawState.mouseButtonPressed, freeDrawState.camera); // under InputHandler.cpp
    }
    TraceLog(LOG_INFO, "freeDrawDRAW called");

}

void freeDrawUnload() {
    freeDrawState.initiliased = false;
    UnloadTransformGizmo();

	// Put texture unload function here
}