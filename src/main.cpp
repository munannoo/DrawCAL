#include "raylib.h"
#include "features/camera/CameraController.h"
#include "rendering/renderer.h"
#include "input/InputHandler.h"
int main()
{
    // Window initialization
    InitWindow(1920, 1080, "DrawCAL");
    SetTargetFPS(60);

    // Camera setup
    Camera3D camera;
    InitCamera(camera);
                             
    // Main loop
    while (!WindowShouldClose())
    {
        if(IsKeyPressed(KEY_F11)){
            ToggleFullscreen();   
        }
        
        UpdateCameraController(camera);

        // Draw everything
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawCameraScene(camera);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}