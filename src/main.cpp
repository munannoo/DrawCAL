#include "main.h"

int currentResIndex = static_cast<int>(resolutionIndex::RES_720p); // Used Enum from toolbar.h for readibility, better than just a 2
int lastResIndex = currentResIndex;

int currentThemeIndex = 0;
int lastThemeIndex = currentThemeIndex;

bool exitWindow = false;

int main()
{   
    SetConfigFlags(FLAG_VSYNC_HINT);       // Enable vsync
    SetConfigFlags(FLAG_MSAA_4X_HINT);     // Enable 4x multisampling anti-aliasing (if available)

    R3D_SetAntiAliasingMode(R3D_ANTI_ALIASING_MODE_FXAA);
    R3D_SetAntiAliasingPreset(R3D_ANTI_ALIASING_PRESET_MEDIUM);

    bool mouseButtonPressed = false; 

    InitWindow(resolutions[currentResIndex].width, resolutions[currentResIndex].height, "DrawCAL"); 

	sceneManagerInit(); // Initialize the scene manager, only needs to be called once

    // Main loop (Runs each frame until the window closes)
    while (!exitWindow)
    {
        if (WindowShouldClose()) exitWindow = true;
        sceneManagerUpdate(); // Update the current scene, also handles scene switching

        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        sceneManagerDraw(); // Draw the current scene

        EndDrawing();
    }

    sceneManagerUnload();

    CloseWindow();

    return 0;
}
