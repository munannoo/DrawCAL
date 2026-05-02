#include "raylib.h"

int main() {
    // 1. Get the current monitor index (usually 0)
    const int monitor = GetCurrentMonitor();

    // 2. Fetch the monitor's native dimensions
    const int screenWidth = GetMonitorWidth(monitor);
    const int screenHeight = GetMonitorHeight(monitor);

    // 3. Initialize window with these dimensions
    InitWindow(screenWidth, screenHeight, "C++ Raylib Fullscreen");

    // Optional: Make it borderless/fullscreen immediately
    ToggleFullscreen();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Running at Native Resolution", 20, 20, 20, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
