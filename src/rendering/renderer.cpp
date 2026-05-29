#include "renderer.h"
#include "objects/object.h"
void DrawCameraScene(const Camera3D &camera)
{
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

    BeginMode3D(camera);
        Cube(cubePosition);
        DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, BLACK);
        DrawGrid(100, 1.0f);
    EndMode3D();
}