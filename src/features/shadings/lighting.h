#pragma once

#include "raylib.h"

#define MAX_SCENE_LIGHTS 16

enum SceneLightType
{
    SCENE_LIGHT_POINT,
    SCENE_LIGHT_DIRECTIONAL
};

struct SceneLight
{
    Vector3 position;
    Vector3 direction;

    Color color;
    float intensity;
    float radius;

    SceneLightType type;
    bool enabled;
    bool isSelected;
};

// Shader lighting
void InitLighting();
void ApplyLightingShader(Model& model);
void UpdateLighting(Camera3D camera);
void UnloadLighting();

// Lighting objects
int CreatePointLight(Vector3 position);
void SetSceneLightPosition(int index, Vector3 position);
void DeleteSceneLight(int index);
void DrawSceneLights();

SceneLight* GetSceneLights();
int GetSceneLightCount();