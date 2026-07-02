#pragma once

#include "raylib.h"

// rPBR's original shader uses 4 lights. DrawCAL can store many scene lights,
// then uploads the nearest active 4 lights to the shader each frame.
#define MAX_SCENE_LIGHTS 512
#define MAX_SHADER_LIGHTS 4

// Matches rPBR pbr.fs light type values.
enum SceneLightType
{
    SCENE_LIGHT_DIRECTIONAL = 0,
    SCENE_LIGHT_POINT = 1
};

struct SceneLight
{
    Vector3 position;
    Vector3 direction;
    Vector3 target;

    Color color;

    // Sent through the alpha channel of the rPBR-style light color.
    float intensity;
    float radius;

    SceneLightType type;

    bool enabled;
    bool isSelected;
};

// Main PBR lighting
void InitLighting();
void ApplyLightingShader(Model& model);
void UpdateLighting(Camera3D camera);
void UnloadLighting();

// Point lights
int CreatePointLight(Vector3 position);
bool CreatePointLightAtMouse(Camera3D camera);
bool CreatePointLightInFrontOfCamera(Camera3D camera);

void DrawSceneLights();

void SetSceneLightPosition(int index, Vector3 position);
void DeleteSceneLight(int index);

SceneLight* GetSceneLights();
int GetSceneLightCount();
void SetSceneLightProperties(int index, Color color, float intensity, float radius);