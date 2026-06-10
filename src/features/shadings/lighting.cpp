#include "lighting.h"
#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <cmath>
#include <objects/object.h>
#define MAX_SHADER_LIGHTS 16

static Shader lightingShader = { 0 };

static int viewPosLoc = -1;

static int ambientColorLoc = -1;
static int lightCountLoc = -1;
static int lightPositionsLoc[MAX_SHADER_LIGHTS];
static int lightColorsLoc[MAX_SHADER_LIGHTS];
static int lightIntensitiesLoc[MAX_SHADER_LIGHTS];
static int lightRadiiLoc[MAX_SHADER_LIGHTS];


// Scene light objects
static SceneLight sceneLights[MAX_SCENE_LIGHTS];
static int sceneLightCount = 0;

void InitLighting()
{
    const char* vsPath = "lighting.vs";
    const char* fsPath = "lighting.fs";

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
    ambientColorLoc = GetShaderLocation(lightingShader, "ambientColor");

    lightCountLoc = GetShaderLocation(lightingShader, "lightCount");

    for (int i = 0; i < MAX_SHADER_LIGHTS; i++)
    {
        lightPositionsLoc[i] = GetShaderLocation(
            lightingShader,
            TextFormat("lightPositions[%i]", i)
        );

        lightColorsLoc[i] = GetShaderLocation(
            lightingShader,
            TextFormat("lightColors[%i]", i)
        );

        lightIntensitiesLoc[i] = GetShaderLocation(
            lightingShader,
            TextFormat("lightIntensities[%i]", i)
        );

        lightRadiiLoc[i] = GetShaderLocation(
            lightingShader,
            TextFormat("lightRadii[%i]", i)
        );
    }
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

    Vector4 ambientColor = { 0.35f, 0.38f, 0.42f, 0.45f };

    SetShaderValue(lightingShader, viewPosLoc, &viewPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(lightingShader, ambientColorLoc, &ambientColor, SHADER_UNIFORM_VEC4);

    int activeLightCount = 0;

    for (int i = 0; i < sceneLightCount && activeLightCount < MAX_SHADER_LIGHTS; i++)
    {
        if (!sceneLights[i].enabled) continue;
        if (sceneLights[i].type != SCENE_LIGHT_POINT) continue;

        Vector3 position = sceneLights[i].position;

        Vector4 color = {
            sceneLights[i].color.r / 255.0f,
            sceneLights[i].color.g / 255.0f,
            sceneLights[i].color.b / 255.0f,
            1.0f
        };

        float intensity = sceneLights[i].intensity;
        float radius = sceneLights[i].radius;

        SetShaderValue(
            lightingShader,
            lightPositionsLoc[activeLightCount],
            &position,
            SHADER_UNIFORM_VEC3
        );

        SetShaderValue(
            lightingShader,
            lightColorsLoc[activeLightCount],
            &color,
            SHADER_UNIFORM_VEC4
        );

        SetShaderValue(
            lightingShader,
            lightIntensitiesLoc[activeLightCount],
            &intensity,
            SHADER_UNIFORM_FLOAT
        );

        SetShaderValue(
            lightingShader,
            lightRadiiLoc[activeLightCount],
            &radius,
            SHADER_UNIFORM_FLOAT
        );

        activeLightCount++;
    }

    SetShaderValue(
        lightingShader,
        lightCountLoc,
        &activeLightCount,
        SHADER_UNIFORM_INT
    );
}

void UnloadLighting()
{
    if (lightingShader.id != 0)
    {
        UnloadShader(lightingShader);
        lightingShader = { 0 };
    }
}

int CreatePointLight(Vector3 position)
{
    if (sceneLightCount >= MAX_SCENE_LIGHTS)
    {
        std::cerr << "Cannot create point light. Maximum scene lights reached." << std::endl;
        return -1;
    }

    SceneLight light = { 0 };

    light.position = position;
    light.direction = { 0.0f, -1.0f, 0.0f };

    light.color = WHITE;
    light.intensity = 4.0f;
    light.radius = 15.0f;

    light.type = SCENE_LIGHT_POINT;
    light.enabled = true;
    light.isSelected = false;

    sceneLights[sceneLightCount] = light;

    int createdIndex = sceneLightCount;
    sceneLightCount++;

    return createdIndex;
}
bool CreatePointLightAtMouse(Camera3D camera)
{
    Vector2 mouse = GetMousePosition();
    Ray ray = GetMouseRay(mouse, camera);

    // Ground/grid plane is y = 0
    if (fabsf(ray.direction.y) < 0.0001f)
    {
        return false;
    }

    float t = -ray.position.y / ray.direction.y;

    if (t < 0.0f)
    {
        return false;
    }

    Vector3 hitPosition = Vector3Add(
        ray.position,
        Vector3Scale(ray.direction, t)
    );

    // Put light above the grid
    hitPosition.y += 3.0f;

    return CreatePointLight(hitPosition);
}

bool CreatePointLightInFrontOfCamera(Camera3D camera)
{
    Vector3 forward = Vector3Normalize(
        Vector3Subtract(camera.target, camera.position)
    );

    Vector3 position = Vector3Add(
        camera.position,
        Vector3Scale(forward, 5.0f)
    );

    return CreatePointLight(position);
}

void DrawSceneLights()
{
    for (int i = 0; i < sceneLightCount; i++)
    {
        if (!sceneLights[i].enabled) continue;

        Color color = sceneLights[i].color;

        if (sceneLights[i].isSelected)
        {
            color = YELLOW;
        }

        Vector3 pos = sceneLights[i].position;

    }
}
void SetSceneLightPosition(int index, Vector3 position)
{
    if (index < 0 || index >= sceneLightCount) return;

    sceneLights[index].position = position;
}
void DeleteSceneLight(int index)
{
    if (index < 0 || index >= sceneLightCount) return;

    for (int i = index; i < sceneLightCount - 1; i++)
    {
        sceneLights[i] = sceneLights[i + 1];
    }

    sceneLightCount--;
}
SceneLight* GetSceneLights()
{
    return sceneLights;
}

int GetSceneLightCount()
{
    return sceneLightCount;
}