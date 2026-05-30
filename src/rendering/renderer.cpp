#include "renderer.h"
#include "objects/object.h"
bool start = false;
void DrawCameraScene(const Camera3D &camera)
{   
    
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    BeginMode3D(camera);
    if(!start){
        start = true;
        Cube(cubePosition);
    }
    DrawGrid(100, 1.0f);
    FrameCube();
    FrameSphere();
    FrameCylinder();
    EndMode3D();
}