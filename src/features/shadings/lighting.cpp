#include "lighting.h"

#include "raylib.h"
#include "raymath.h"

#include <cmath>
#include <float.h>
#include <iostream>

std::vector<std::unique_ptr<Light>> lights;

// Light implementation

unsigned int Light::nextId = 0;

Light::Light(Vector3 position, Vector3 target, Color Color, R3D_LightType type)
{
    id = nextId++;
    selected = false;

    light = R3D_CreateLight(type);
    R3D_LightLookAt(light, position, target);
    R3D_EnableShadow(light);
    R3D_SetLightActive(light, true);
}

Light::~Light()
{
    if (R3D_IsLightExist(light))
    {
        R3D_DestroyLight(light);
    }
}

R3D_Light Light::getLight() const
{
    return light;
}

unsigned int Light::getId() const
{
    return id;
}

bool Light::getSelected() const
{
    return selected;
}

void Light::setSelected(bool value)
{
    selected = value;
}

void initialiseEnvironment() {

    // Set ambient light
    //R3D_ENVIRONMENT_SET(ambient.color, Color{ 10, 10, 10, 255 });

    R3D_Environment* env = R3D_GetEnvironment();

    // Core workspace rendering
    //env->background.color = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
    //env->background.energy = 1.0f;
    env->background.color = Color{ 45, 48, 52, 255 };

    env->ambient.color = Color{ 20, 20, 20, 255 };
    env->ambient.energy = 0.3f;

    // Ambient Occlusion
    env->ssao.enabled = true;
    env->ssao.intensity = 0.7f;
    env->ssao.radius = 0.5f;
    env->ssao.bias = 0.03f;

    // Indirect lighting
    env->ssil.enabled = true;

    // Global Illumination, simulates light bounces
	env->ssgi.enabled = false;

	// Screen space reflections
	env->ssr.enabled = false;

    //// Geometry readability
    //R3D_EnvSSAO
    //// HDR display


    R3D_SetEnvironment(env);
}

//
//struct ShaderLightLocations
//{
//    int enabledLoc = -1;
//    int typeLoc = -1;
//    int positionLoc = -1;
//    int targetLoc = -1;
//    int colorLoc = -1;
//    int radiusLoc = -1;
//};
//
//static Shader lightingShader = { 0 };
//static SceneLight sceneLights[MAX_SCENE_LIGHTS];
//static int sceneLightCount = 0;
//
//static ShaderLightLocations lightLocs[MAX_SHADER_LIGHTS];
//static int viewPosLoc = -1;
//static int renderModeLoc = -1;
//static int ambientColorLoc = -1;
//static int heightScaleLoc = -1;
//static int useParallaxMappingLoc = -1;
//
//static bool legacyRayTracingEnabled = false;
//
//static void DisableUnusedShaderLights(int firstSlot)
//{
//    for (int i = firstSlot; i < MAX_SHADER_LIGHTS; i++)
//    {
//        int enabled = 0;
//        SetShaderValue(lightingShader, lightLocs[i].enabledLoc, &enabled, SHADER_UNIFORM_INT);
//    }
//}
//
//static void UploadLightToShader(int shaderSlot, const SceneLight& light)
//{
//    int enabled = light.enabled ? 1 : 0;
//    int type = (int)light.type;
//
//    Vector3 position = light.position;
//    Vector3 target = light.target;
//
//    Vector4 color = {
//        light.color.r / 255.0f,
//        light.color.g / 255.0f,
//        light.color.b / 255.0f,
//        light.intensity
//    };
//
//    SetShaderValue(lightingShader, lightLocs[shaderSlot].enabledLoc, &enabled, SHADER_UNIFORM_INT);
//    SetShaderValue(lightingShader, lightLocs[shaderSlot].typeLoc, &type, SHADER_UNIFORM_INT);
//    SetShaderValue(lightingShader, lightLocs[shaderSlot].positionLoc, &position, SHADER_UNIFORM_VEC3);
//    SetShaderValue(lightingShader, lightLocs[shaderSlot].targetLoc, &target, SHADER_UNIFORM_VEC3);
//    float radius = fmaxf(light.radius, 0.01f);
//
//    SetShaderValue(lightingShader, lightLocs[shaderSlot].colorLoc, &color, SHADER_UNIFORM_VEC4);
//    SetShaderValue(lightingShader, lightLocs[shaderSlot].radiusLoc, &radius, SHADER_UNIFORM_FLOAT);
//}
//
//void InitLighting()
//{
//    const char* vsPath = "shaders/lighting.vs";
//    const char* fsPath = "shaders/lighting.fs";
//
//    if (!FileExists(vsPath))
//    {
//        std::cerr << "Missing vertex shader: " << vsPath << std::endl;
//    }
//
//    if (!FileExists(fsPath))
//    {
//        std::cerr << "Missing fragment shader: " << fsPath << std::endl;
//    }
//
//    lightingShader = LoadShader(vsPath, fsPath);
//
//    // Raylib standard shader locations used by DrawModel().
//    lightingShader.locs[SHADER_LOC_MATRIX_MVP] =
//        GetShaderLocation(lightingShader, "mvp");
//
//    lightingShader.locs[SHADER_LOC_MATRIX_MODEL] =
//        GetShaderLocation(lightingShader, "matModel");
//
//    lightingShader.locs[SHADER_LOC_MATRIX_NORMAL] =
//        GetShaderLocation(lightingShader, "matNormal");
//
//    lightingShader.locs[SHADER_LOC_MAP_ALBEDO] =
//        GetShaderLocation(lightingShader, "albedoMap");
//
//    lightingShader.locs[SHADER_LOC_MAP_NORMAL] =
//        GetShaderLocation(lightingShader, "normalMap");
//
//    lightingShader.locs[SHADER_LOC_MAP_METALNESS] =
//        GetShaderLocation(lightingShader, "metalnessMap");
//
//    lightingShader.locs[SHADER_LOC_MAP_ROUGHNESS] =
//        GetShaderLocation(lightingShader, "roughnessMap");
//
//    lightingShader.locs[SHADER_LOC_MAP_OCCLUSION] =
//        GetShaderLocation(lightingShader, "aoMap");
//
//    lightingShader.locs[SHADER_LOC_MAP_HEIGHT] =
//        GetShaderLocation(lightingShader, "heightMap");
//
//    lightingShader.locs[SHADER_LOC_COLOR_DIFFUSE] =
//        GetShaderLocation(lightingShader, "colDiffuse");
//
//    viewPosLoc = GetShaderLocation(lightingShader, "viewPos");
//    renderModeLoc = GetShaderLocation(lightingShader, "renderMode");
//    ambientColorLoc = GetShaderLocation(lightingShader, "ambientColor");
//    heightScaleLoc = GetShaderLocation(lightingShader, "heightScale");
//    useParallaxMappingLoc = GetShaderLocation(lightingShader, "useParallaxMapping");
//
//    for (int i = 0; i < MAX_SHADER_LIGHTS; i++)
//    {
//        lightLocs[i].enabledLoc = GetShaderLocation(lightingShader, TextFormat("lights[%i].enabled", i));
//        lightLocs[i].typeLoc = GetShaderLocation(lightingShader, TextFormat("lights[%i].type", i));
//        lightLocs[i].positionLoc = GetShaderLocation(lightingShader, TextFormat("lights[%i].position", i));
//        lightLocs[i].targetLoc = GetShaderLocation(lightingShader, TextFormat("lights[%i].target", i));
//        lightLocs[i].colorLoc = GetShaderLocation(lightingShader, TextFormat("lights[%i].color", i));
//        lightLocs[i].radiusLoc = GetShaderLocation(lightingShader, TextFormat("lights[%i].radius", i));
//    }
//
//    int renderMode = 0;
//    int useParallaxMapping = 0;
//    float heightScale = 0.0f;
//
//    SetShaderValue(lightingShader, renderModeLoc, &renderMode, SHADER_UNIFORM_INT);
//    SetShaderValue(lightingShader, useParallaxMappingLoc, &useParallaxMapping, SHADER_UNIFORM_INT);
//    SetShaderValue(lightingShader, heightScaleLoc, &heightScale, SHADER_UNIFORM_FLOAT);
//
//    DisableUnusedShaderLights(0);
//}
//
//void ApplyLightingShader(Model& model)
//{
//    for (int i = 0; i < model.materialCount; i++)
//    {
//        model.materials[i].shader = lightingShader;
//    }
//}
//
//void UpdateLighting(Camera3D camera)
//{
//    if (lightingShader.id == 0) return;
//
//    Vector3 viewPos = camera.position;
//    // rPBR normally gets extra brightness from HDR environment maps.
//    // DrawCAL currently uses direct lights only, so use a brighter ambient fallback.
//    Vector4 ambientColor = { 0.22f, 0.22f, 0.24f, 1.0f };
//    int renderMode = 0;
//    int useParallaxMapping = 0;
//    float heightScale = 0.0f;
//
//    SetShaderValue(lightingShader, viewPosLoc, &viewPos, SHADER_UNIFORM_VEC3);
//    SetShaderValue(lightingShader, ambientColorLoc, &ambientColor, SHADER_UNIFORM_VEC4);
//    SetShaderValue(lightingShader, renderModeLoc, &renderMode, SHADER_UNIFORM_INT);
//    SetShaderValue(lightingShader, useParallaxMappingLoc, &useParallaxMapping, SHADER_UNIFORM_INT);
//    SetShaderValue(lightingShader, heightScaleLoc, &heightScale, SHADER_UNIFORM_FLOAT);
//
//    bool usedLights[MAX_SCENE_LIGHTS] = { false };
//    int uploadedCount = 0;
//
//    // Upload nearest active lights first. This keeps DrawCAL able to store many
//    // lights while staying compatible with rPBR's 4-light shader limit.
//    for (int slot = 0; slot < MAX_SHADER_LIGHTS; slot++)
//    {
//        int bestIndex = -1;
//        float bestDistance = FLT_MAX;
//
//        for (int i = 0; i < sceneLightCount; i++)
//        {
//            if (usedLights[i]) continue;
//            if (!sceneLights[i].enabled) continue;
//
//            float distanceToCamera = Vector3DistanceSqr(sceneLights[i].position, camera.position);
//
//            if (distanceToCamera < bestDistance)
//            {
//                bestDistance = distanceToCamera;
//                bestIndex = i;
//            }
//        }
//
//        if (bestIndex < 0) break;
//
//        usedLights[bestIndex] = true;
//        UploadLightToShader(slot, sceneLights[bestIndex]);
//        uploadedCount++;
//    }
//
//    DisableUnusedShaderLights(uploadedCount);
//}
//
//void UnloadLighting()
//{
//    if (lightingShader.id != 0)
//    {
//        UnloadShader(lightingShader);
//        lightingShader = { 0 };
//    }
//}
//
//void ClearSceneLights()
//{
//    sceneLightCount = 0;
//    if (lightingShader.id != 0) DisableUnusedShaderLights(0);
//}
//
//int CreatePointLight(Vector3 position)
//{
//    if (sceneLightCount >= MAX_SCENE_LIGHTS)
//    {
//        std::cerr << "Cannot create point light. Maximum scene lights reached." << std::endl;
//        return -1;
//    }
//
//    SceneLight light = { 0 };
//
//    light.position = position;
//    light.direction = { 0.0f, -1.0f, 0.0f };
//    light.target = { 0.0f, 0.0f, 0.0f };
//
//    light.color = WHITE;
//    light.intensity = 350.0f;
//    light.radius = 35.0f;
//
//    light.type = SCENE_LIGHT_POINT;
//    light.enabled = true;
//    light.isSelected = false;
//
//    sceneLights[sceneLightCount] = light;
//
//    int createdIndex = sceneLightCount;
//    sceneLightCount++;
//
//    return createdIndex;
//}
//
//bool CreatePointLightAtMouse(Camera3D camera)
//{
//    Vector2 mouse = GetMousePosition();
//    Ray ray = GetScreenToWorldRay(mouse, camera);
//
//    if (fabsf(ray.direction.y) < 0.0001f)
//    {
//        return false;
//    }
//
//    float t = -ray.position.y / ray.direction.y;
//
//    if (t < 0.0f)
//    {
//        return false;
//    }
//
//    Vector3 hitPosition = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
//    hitPosition.y += 3.0f;
//
//    return CreatePointLight(hitPosition) != -1;
//}
//
//bool CreatePointLightInFrontOfCamera(Camera3D camera)
//{
//    Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
//    Vector3 position = Vector3Add(camera.position, Vector3Scale(forward, 5.0f));
//
//    return CreatePointLight(position) != -1;
//}
//
//void DrawSceneLights()
//{
//    for (int i = 0; i < sceneLightCount; i++)
//    {
//        if (!sceneLights[i].enabled) continue;
//
//        Color color = sceneLights[i].isSelected ? YELLOW : sceneLights[i].color;
//        Vector3 pos = sceneLights[i].position;
//
//        DrawSphere(pos, 0.18f, color);
//        DrawSphereWires(pos, 0.22f, 8, 8, WHITE);
//    }
//}
//
//void SetSceneLightPosition(int index, Vector3 position)
//{
//    if (index < 0 || index >= sceneLightCount) return;
//
//    sceneLights[index].position = position;
//}
//
//void SetSceneLightProperties(int index, Color color, float intensity, float radius)
//{
//    if (index < 0 || index >= sceneLightCount) return;
//
//    sceneLights[index].color = color;
//    sceneLights[index].intensity = fmaxf(intensity, 0.0f);
//    sceneLights[index].radius = fmaxf(radius, 0.01f);
//}
//
//void DeleteSceneLight(int index)
//{
//    if (index < 0 || index >= sceneLightCount) return;
//
//    for (int i = index; i < sceneLightCount - 1; i++)
//    {
//        sceneLights[i] = sceneLights[i + 1];
//    }
//
//    sceneLightCount--;
//}
//
//SceneLight* GetSceneLights()
//{
//    return sceneLights;
//}
//
//int GetSceneLightCount()
//{
//    return sceneLightCount;
//}
//
//void SetRayTracingEnabled(bool enabled)
//{
//    legacyRayTracingEnabled = false;
//
//    if (enabled)
//    {
//        TraceLog(LOG_WARNING, "Ray tracing was removed. DrawCAL is using rPBR-style PBR lighting instead.");
//    }
//}
//
//bool ToggleRayTracing()
//{
//    SetRayTracingEnabled(!legacyRayTracingEnabled);
//    return false;
//}
//
//bool IsRayTracingEnabled()
//{
//    return false;
//}
//
//void BeginRayTraceSceneUpload() {}
//void AddRayTraceCube(Vector3 position, Vector3 rotation, Vector3 scale) { (void)position; (void)rotation; (void)scale; }
//void AddRayTraceSphere(Vector3 position, Vector3 scale) { (void)position; (void)scale; }
//void AddRayTraceCylinder(Vector3 position, Vector3 rotation, Vector3 scale) { (void)position; (void)rotation; (void)scale; }
//void EndRayTraceSceneUpload() {}
//
//Shader GetShadowShader()
//{
//    return Shader{ 0 };
//}
//
//bool HasShadowCastingPointLight()
//{
//    return false;
//}
//
//int GetPointShadowFaceCount()
//{
//    return 0;
//}
//
//void BeginPointShadowMapFace(int face)
//{
//    (void)face;
//}
//
//void EndPointShadowMapFace() {}
//
//Camera3D GetPointShadowCamera(int face)
//{
//    (void)face;
//    return Camera3D{ 0 };
//}
//
//void BindPointShadowMaps() {}
