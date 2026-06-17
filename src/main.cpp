#include "raylib.h"
#include "features/shadings/textures.h"
#include "features/camera/CameraController.h"
#include "rendering/renderer.h"
#include "input/InputHandler.h"
#include "ui/panels/toolbar.h"
#include "ui/widgets/buttons.h"
#include "objects/object.h"
#include "features/manipulation/Transform.h"
#include "features/shadings/lighting.h"
int main()
{   
    SetConfigFlags(FLAG_VSYNC_HINT); // Enable vsync
    SetConfigFlags(FLAG_MSAA_4X_HINT); // Enable 4x multisampling anti-aliasing (if available)
    // Resolution settings determined inside toolbar.cpp
    TraceLog(LOG_INFO, "Working directory: %s", GetWorkingDirectory());
    int currentResIndex = RES_720p; // Used Enum from toolbar.h for readibility, better than just a 2
    int lastResIndex = currentResIndex;
    bool dropdownEditmode = false;
    bool mouseButtonPressed = false; 
    const Color darkBackground = { 57, 57, 57, 255 };
    InitWindow(cr[currentResIndex].width, cr[currentResIndex].height, "DrawCAL"); 
    initModels(); // Initialize the Models, only needs to be called once
    InitTransformGizmo(); // Initialize the transform gizmo, only needs to be called once
    Camera3D camera;
    InitCamera(camera);
    initgridShader();
    // Main loop (Runs each frame until the window closes)
    while (!WindowShouldClose())
    {
        Ray ray = GetMouseRay(GetMousePosition(), camera);
        bool usingGizmo = updateObjectTransformGizmo(camera);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !usingGizmo)
        {
            leftclick(ray);
        }
        if (IsKeyPressed(KEY_F11))
        {
            ToggleFullscreen();
            ShowCursor();
        }
        if (currentResIndex != lastResIndex)
        {
            SetWindowSize(cr[currentResIndex].width, cr[currentResIndex].height);
            lastResIndex = currentResIndex;
        }
        if (!usingGizmo)
        {
            UpdateCameraController(camera);
        }
        UpdateLighting(camera);

        if (HasShadowCastingPointLight())
        {
            for (int face = 0; face < GetPointShadowFaceCount(); face++)
            {
                BeginPointShadowMapFace(face);

                Camera3D shadowCamera = GetPointShadowCamera(face);

                BeginMode3D(shadowCamera);
                    DrawSceneForShadowMap();
                EndMode3D();

                EndPointShadowMapFace();
            }
        }

        BeginDrawing();
        ClearBackground(darkBackground);
        BindPointShadowMaps();
        DrawCameraScene(camera);
        topBar(currentResIndex, dropdownEditmode);
        if (IsKeyPressed(KEY_BACKSPACE))
        {
            deleteobj();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || mouseButtonPressed)
        {
            contextMenu(mouseButtonPressed, camera);
        }
        
        EndDrawing();
    }
    UnloadTransformGizmo();
    Unload();
    CloseWindow();
    return 0;
}