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
    // Resolution settings determined inside toolbar.cpp

    bool mouseButtonPressed = false; 

    InitWindow(resolutions[currentResIndex].width, resolutions[currentResIndex].height, "DrawCAL"); 

	sceneManagerInit(); // Initialize the scene manager, only needs to be called once

    initModels(); // Initialize the Models, only needs to be called once
    InitTransformGizmo();
    initgridShader();

    // Main loop (Runs each frame until the window closes)
    while (!exitWindow)
    {
        // Much of it is obsolete, need to incorporate the changes into the individual functions, initilaise, update whatnot
        // Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
        //bool usingGizmo = updateObjectTransformGizmo(camera);

        //if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !usingGizmo)
        //{
        //    leftclick(ray);
        //}

        //if (IsKeyPressed(KEY_F11))
        //{
        //    ToggleFullscreen();
        //    ShowCursor();
        //}

        //if (currentResIndex != lastResIndex)
        //{
        //    SetWindowSize(cr[currentResIndex].width, cr[currentResIndex].height);
        //    lastResIndex = currentResIndex;
        //}

        //if (!usingGizmo)
        //{
        //    UpdateCameraController(camera);
        //}

        // rPBR-style PBR lighting uniforms are updated before the scene is drawn.
        //UpdateLighting(camera);

        if (WindowShouldClose()) exitWindow = true;
        sceneManagerUpdate(); // Update the current scene, also handles scene switching

        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        sceneManagerDraw(); // Draw the current scene

        // Incorporate into the other stuff
        //if (IsKeyPressed(KEY_BACKSPACE))
        //{
        //    deleteobj();
        //}

        //if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || mouseButtonPressed)
        //{
        //    contextMenu(mouseButtonPressed, camera);
        //}

        EndDrawing();
    }
    // we do have an unload function
    //UnloadTransformGizmo();
    Unload();
    CloseWindow();

    return 0;
}
