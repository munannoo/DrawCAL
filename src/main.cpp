#include "raylib.h"
#include "features/camera/CameraController.h"
#include "rendering/renderer.h"
#include "input/InputHandler.h"
#include "ui/panels/toolbar.h"
#include "ui/widgets/buttons.h"
#include "objects/object.h"
int main()
{   
    SetConfigFlags(FLAG_VSYNC_HINT); // Enable vsync
    // Resolution settings determined inside toolbar.cpp
    
    int currentResIndex = RES_720p; // Used Enum from toolbar.h for readibility, better than just a 2
    int lastResIndex = currentResIndex;
    bool dropdownEditmode = false;
    bool mouseButtonPressed = false; 

    InitWindow(cr[currentResIndex].width, cr[currentResIndex].height, "DrawCAL"); 
    initModels(); // Initialize the models for the objects, only needs to be called once
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
        topBar(currentResIndex, dropdownEditmode);

        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || mouseButtonPressed){
            contextMenu(mouseButtonPressed, camera); // under InputHandler.cpp
        }
        
        EndDrawing();
    }
    Unload();
    CloseWindow();
    return 0;
}