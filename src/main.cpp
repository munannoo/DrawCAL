#include "raylib.h"
#include "features/camera/CameraController.h"

int main()
{
    // Window initialization
    InitWindow(800, 600, "Camera Controller Example");
    SetTargetFPS(60);

    // Camera setup
    Camera3D camera;
    InitCamera(camera);

    // Main loop
    while (!WindowShouldClose())
    {
        // Update camera logic
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