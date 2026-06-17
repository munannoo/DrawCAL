#pragma once

#include "raylib.h"

#define MAX_SCENE_LIGHTS 64

enum SceneLightType
{
    SCENE_LIGHT_POINT = 0
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

// 360 point-light shadow maps
Shader GetShadowShader();

bool HasShadowCastingPointLight();

int GetPointShadowFaceCount();

void BeginPointShadowMapFace(int face);
void EndPointShadowMapFace();

Camera3D GetPointShadowCamera(int face);

void BindPointShadowMaps();