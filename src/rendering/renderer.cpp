#include "renderer.h"
void DrawCameraScene(const Camera3D &camera)
{
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

    BeginMode3D(camera);
        DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
        DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, BLACK);
        
        DrawGrid(100, 1.0f);
    EndMode3D();
}