#include "main.h"

int currentResIndex = static_cast<int>(resolutionIndex::RES_720p); // Used Enum from toolbar.h for readibility, better than just a 2
int lastResIndex = currentResIndex;

int main()
{   
    SetConfigFlags(FLAG_VSYNC_HINT); // Enable vsync
    // Resolution settings determined inside toolbar.cpp
    
    bool dropdownEditmode = false;
    bool mouseButtonPressed = false; 

    InitWindow(resolutions[currentResIndex].width, resolutions[currentResIndex].height, "DrawCAL"); 

	sceneManagerInit(); // Initialize the scene manager, only needs to be called once

    initModels(); // Initialize the Models, only needs to be called once
               
    // Main loop (Runs each frame until the window closes)
    while (!WindowShouldClose())
    {
		sceneManagerUpdate(); // Update the current scene, also handles scene switching



        BeginDrawing();
        ClearBackground(RAYWHITE);
        sceneManagerDraw(); // Draw the current scene



        
        EndDrawing();
    }
    Unload();
    CloseWindow();
    return 0;
}