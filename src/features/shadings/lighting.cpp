#include "lighting.h"
#include "raylib.h"
#include <iostream>

static Shader lightingShader = { 0 };

static int viewPosLoc = -1;
static int lightDirLoc = -1;
static int lightColorLoc = -1;
static int ambientColorLoc = -1;

void InitLighting()
{
    const char* vsPath = "../../assets/models/lighting.vs";
    const char* fsPath = "../../assets/models/lighting.fs";

    if (!FileExists(vsPath))
    {
        std::cerr << "Missing vertex shader: " << vsPath << std::endl;
    }

    if (!FileExists(fsPath))
    {
        std::cerr << "Missing fragment shader: " << fsPath << std::endl;
    }

    lightingShader = LoadShader(vsPath, fsPath);

    lightingShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(lightingShader, "mvp");
    lightingShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(lightingShader, "matModel");
    lightingShader.locs[SHADER_LOC_MATRIX_NORMAL] = GetShaderLocation(lightingShader, "matNormal");

    lightingShader.locs[SHADER_LOC_MAP_DIFFUSE] = GetShaderLocation(lightingShader, "texture0");
    lightingShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(lightingShader, "colDiffuse");

    viewPosLoc = GetShaderLocation(lightingShader, "viewPos");
    lightDirLoc = GetShaderLocation(lightingShader, "lightDir");
    lightColorLoc = GetShaderLocation(lightingShader, "lightColor");
    ambientColorLoc = GetShaderLocation(lightingShader, "ambientColor");
}

void ApplyLightingShader(Model& model)
{
    if (model.materialCount <= 0) return;

    model.materials[0].shader = lightingShader;
}

void UpdateLighting(Camera3D camera)
{
    Vector3 viewPos = camera.position;

    // Blender-like viewport lighting
    Vector3 lightDir = { -0.35f, -1.0f, -0.45f };
    Vector4 lightColor = { 1.0f, 0.96f, 0.90f, 0.80f };
    Vector4 ambientColor = { 0.55f, 0.60f, 0.70f, 0.45f };

    SetShaderValue(lightingShader, viewPosLoc, &viewPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(lightingShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(lightingShader, lightColorLoc, &lightColor, SHADER_UNIFORM_VEC4);
    SetShaderValue(lightingShader, ambientColorLoc, &ambientColor, SHADER_UNIFORM_VEC4);
}

void UnloadLighting()
{
    if (lightingShader.id != 0)
    {
        UnloadShader(lightingShader);
        lightingShader = { 0 };
    }
}