#pragma once

#include "raylib.h"

// CPU-side scene can store a lot of lights. The shader still receives the
// nearest/most relevant MAX_SHADER_LIGHTS lights each frame.
#define MAX_SCENE_LIGHTS 512
#define MAX_RAYTRACE_OBJECTS 64

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

// Ray-traced shadow toggle. This is a GLSL ray query path for the current
// DrawCAL primitives. It supports all uploaded shader lights, not only the
// first shadow-map light.
void SetRayTracingEnabled(bool enabled);
bool ToggleRayTracing();
bool IsRayTracingEnabled();

// CPU -> shader object upload used by object.cpp every frame.
void BeginRayTraceSceneUpload();
void AddRayTraceCube(Vector3 position, Vector3 rotation, Vector3 scale);
void AddRayTraceSphere(Vector3 position, Vector3 scale);
void AddRayTraceCylinder(Vector3 position, Vector3 rotation, Vector3 scale);
void EndRayTraceSceneUpload();

// Point lights
int CreatePointLight(Vector3 position);
bool CreatePointLightAtMouse(Camera3D camera);
bool CreatePointLightInFrontOfCamera(Camera3D camera);

void DrawSceneLights();

void SetSceneLightPosition(int index, Vector3 position);
void DeleteSceneLight(int index);

SceneLight* GetSceneLights();
int GetSceneLightCount();

// Legacy 360 point-light shadow maps. Used only when ray tracing is OFF.
Shader GetShadowShader();

bool HasShadowCastingPointLight();

int GetPointShadowFaceCount();

void BeginPointShadowMapFace(int face);
void EndPointShadowMapFace();

Camera3D GetPointShadowCamera(int face);

void BindPointShadowMaps();
