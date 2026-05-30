#include "raylib.h"
#include "features/camera/CameraController.h"
#include "rendering/renderer.h"
#include "input/InputHandler.h"
#include "ui/panels/toolbar.h"
#include "ui/widgets/buttons.h"

int main()
{   
    int currentResIndex = 2;
    int lastResIndex = currentResIndex;
    bool dropdownEditmode = false;
    int r = 0;
    InitWindow(cr[currentResIndex].width, cr[currentResIndex].height, "DrawCAL"); 
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
    Camera3D camera;
    InitCamera(camera);             
    // Main loop (Runs each frame until the window closes)
    while (!WindowShouldClose())
    {
        if(IsKeyPressed(KEY_F11)){
            ToggleFullscreen();
            ShowCursor();
        }
        if (currentResIndex != lastResIndex) {
            SetWindowSize(cr[currentResIndex].width, cr[currentResIndex].height);
            lastResIndex = currentResIndex;
        }
        UpdateCameraController(camera);
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawCameraScene(camera);
        topbar(currentResIndex, dropdownEditmode);
        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || r){
            rightclick(r, camera);
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}