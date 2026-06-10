#include "renderer.h"
#include "raymath.h"
#include "rlgl.h"
#include "objects/object.h"
Shader gridShader;
Model gridPlane;
bool start = false;
int camPosLoc;
int gridColorLoc;
int farGridColorLoc;
int scaleLoc;
void initgridShader() 
{
    TraceLog(LOG_INFO, "grid.vs exists: %s", FileExists("../../assets/models/grid.vs") ? "YES" : "NO");
    TraceLog(LOG_INFO, "grid.fs exists: %s", FileExists("../../assets/models/grid.fs") ? "YES" : "NO");

    gridShader = LoadShader("../../assets/models/grid.vs","../../assets/models/grid.fs");

    gridPlane = LoadModelFromMesh(GenMeshPlane(2000.0f, 2000.0f, 1, 1));
    gridPlane.materials[0].shader = gridShader;

    camPosLoc = GetShaderLocation(gridShader, "cameraPos");
    gridColorLoc = GetShaderLocation(gridShader, "gridColor");
    farGridColorLoc = GetShaderLocation(gridShader, "farGridColor");
    scaleLoc = GetShaderLocation(gridShader, "scale");
}

Vector4 gridCol = { 0.5f, 0.5f, 0.5f, 0.8f };
Vector4 farGridCol = { 0.2f, 0.2f, 0.2f, 0.2f };
float scale = 1.0f;
void DrawCameraScene(const Camera3D &camera)
{
    Vector3 camPos = camera.position;

    SetShaderValue(gridShader, camPosLoc, &camPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(gridShader, gridColorLoc, &gridCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(gridShader, farGridColorLoc, &farGridCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(gridShader, scaleLoc, &scale, SHADER_UNIFORM_FLOAT);

    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

    BeginMode3D(camera);

        rlDisableBackfaceCulling();

        BeginBlendMode(BLEND_ALPHA);
            DrawModel(gridPlane, { camera.position.x, 0.0f, camera.position.z }, 1.0f, WHITE);
        EndBlendMode();
        
        rlEnableBackfaceCulling();
        if (!start)
        {
            start = true;
            cube(cubePosition);
        }
        frameCube();
        frameSphere();
        frameCylinder();
        drawObjectTransformGizmo();

    EndMode3D();
}