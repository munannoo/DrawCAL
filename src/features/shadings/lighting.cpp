#include "lighting.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <iostream>
#include <cmath>
#include <float.h>

#define MAX_SHADER_LIGHTS 64

static Shader lightingShader = { 0 };
static Shader shadowShader = { 0 };

static int viewPosLoc = -1;
static int ambientColorLoc = -1;
static int heightScaleLoc = -1;
static int useParallaxMappingLoc = -1;

static int lightCountLoc = -1;
static int lightPositionsLoc[MAX_SHADER_LIGHTS];
static int lightColorsLoc[MAX_SHADER_LIGHTS];
static int lightIntensitiesLoc[MAX_SHADER_LIGHTS];
static int lightRadiiLoc[MAX_SHADER_LIGHTS];

static int rayTracingEnabledLoc = -1;

static int rtCubeCountLoc = -1;
static int rtCubePositionsLoc[MAX_RAYTRACE_OBJECTS];
static int rtCubeRotationsLoc[MAX_RAYTRACE_OBJECTS];
static int rtCubeScalesLoc[MAX_RAYTRACE_OBJECTS];

static int rtSphereCountLoc = -1;
static int rtSpherePositionsLoc[MAX_RAYTRACE_OBJECTS];
static int rtSphereScalesLoc[MAX_RAYTRACE_OBJECTS];

static int rtCylinderCountLoc = -1;
static int rtCylinderPositionsLoc[MAX_RAYTRACE_OBJECTS];
static int rtCylinderRotationsLoc[MAX_RAYTRACE_OBJECTS];
static int rtCylinderScalesLoc[MAX_RAYTRACE_OBJECTS];

static bool rayTracingEnabled = true;

static Vector3 rtCubePositions[MAX_RAYTRACE_OBJECTS];
static Vector3 rtCubeRotations[MAX_RAYTRACE_OBJECTS];
static Vector3 rtCubeScales[MAX_RAYTRACE_OBJECTS];
static int rtCubeCount = 0;

static Vector3 rtSpherePositions[MAX_RAYTRACE_OBJECTS];
static Vector3 rtSphereScales[MAX_RAYTRACE_OBJECTS];
static int rtSphereCount = 0;

static Vector3 rtCylinderPositions[MAX_RAYTRACE_OBJECTS];
static Vector3 rtCylinderRotations[MAX_RAYTRACE_OBJECTS];
static Vector3 rtCylinderScales[MAX_RAYTRACE_OBJECTS];
static int rtCylinderCount = 0;

static SceneLight sceneLights[MAX_SCENE_LIGHTS];
static int sceneLightCount = 0;

// 360 point-light shadows
static const int POINT_SHADOW_SIZE = 2048;
static const float POINT_SHADOW_NEAR = 0.1f;
static const float POINT_SHADOW_FAR = 150.0f;

// Shadows appear near the editor camera and fade when the camera is far.
static const float SHADOW_CAMERA_MAX_DISTANCE = 80.0f;
static const float SHADOW_CAMERA_FADE_RANGE = 30.0f;

static const int POINT_SHADOW_TEXTURE_SLOT_START = 10;

struct PointShadowMapData
{
    unsigned int fbo[6] = { 0 };
    Texture2D depth[6] = { 0 };
};

static PointShadowMapData pointShadowMapData;

static int pointLightSpaceMatrixLoc = -1;

static int usePointLightShadowLoc = -1;
static int shadowPointLightIndexLoc = -1;
static int shadowPointLightPositionLoc = -1;
static int shadowCameraMaxDistanceLoc = -1;
static int shadowCameraFadeRangeLoc = -1;

static int pointShadowMapLoc[6] = { -1, -1, -1, -1, -1, -1 };
static int pointLightSpaceMatricesLoc[6] = { -1, -1, -1, -1, -1, -1 };

void SetRayTracingEnabled(bool enabled)
{
    rayTracingEnabled = enabled;
    TraceLog(LOG_INFO, "Ray traced shadows: %s", rayTracingEnabled ? "ON" : "OFF");
}

bool ToggleRayTracing()
{
    SetRayTracingEnabled(!rayTracingEnabled);
    return rayTracingEnabled;
}

bool IsRayTracingEnabled()
{
    return rayTracingEnabled;
}

void BeginRayTraceSceneUpload()
{
    rtCubeCount = 0;
    rtSphereCount = 0;
    rtCylinderCount = 0;
}

void AddRayTraceCube(Vector3 position, Vector3 rotation, Vector3 scale)
{
    if (rtCubeCount >= MAX_RAYTRACE_OBJECTS) return;

    rtCubePositions[rtCubeCount] = position;
    rtCubeRotations[rtCubeCount] = rotation;
    rtCubeScales[rtCubeCount] = scale;
    rtCubeCount++;
}

void AddRayTraceSphere(Vector3 position, Vector3 scale)
{
    if (rtSphereCount >= MAX_RAYTRACE_OBJECTS) return;

    rtSpherePositions[rtSphereCount] = position;
    rtSphereScales[rtSphereCount] = scale;
    rtSphereCount++;
}

void AddRayTraceCylinder(Vector3 position, Vector3 rotation, Vector3 scale)
{
    if (rtCylinderCount >= MAX_RAYTRACE_OBJECTS) return;

    rtCylinderPositions[rtCylinderCount] = position;
    rtCylinderRotations[rtCylinderCount] = rotation;
    rtCylinderScales[rtCylinderCount] = scale;
    rtCylinderCount++;
}

void EndRayTraceSceneUpload()
{
    // Kept as a named end-step so object.cpp can clearly bracket the upload.
}

static bool GetPrimaryShadowPointLight(Vector3* outPosition, int* outActiveLightIndex)
{
    int activeIndex = 0;

    for (int i = 0; i < sceneLightCount; i++)
    {
        if (!sceneLights[i].enabled) continue;
        if (sceneLights[i].type != SCENE_LIGHT_POINT) continue;

        if (outPosition != nullptr)
        {
            *outPosition = sceneLights[i].position;
        }

        if (outActiveLightIndex != nullptr)
        {
            *outActiveLightIndex = activeIndex;
        }

        return true;
    }

    return false;
}

static Matrix GetPointLightSpaceMatrix(int face)
{
    Vector3 lightPos = { 0.0f, 0.0f, 0.0f };
    int activeIndex = -1;

    if (!GetPrimaryShadowPointLight(&lightPos, &activeIndex))
    {
        return MatrixIdentity();
    }

    Vector3 target = lightPos;
    Vector3 up = { 0.0f, -1.0f, 0.0f };

    switch (face)
    {
        case 0: // +X
            target = Vector3Add(lightPos, { 1.0f, 0.0f, 0.0f });
            up = { 0.0f, -1.0f, 0.0f };
            break;

        case 1: // -X
            target = Vector3Add(lightPos, { -1.0f, 0.0f, 0.0f });
            up = { 0.0f, -1.0f, 0.0f };
            break;

        case 2: // +Y
            target = Vector3Add(lightPos, { 0.0f, 1.0f, 0.0f });
            up = { 0.0f, 0.0f, 1.0f };
            break;

        case 3: // -Y
            target = Vector3Add(lightPos, { 0.0f, -1.0f, 0.0f });
            up = { 0.0f, 0.0f, -1.0f };
            break;

        case 4: // +Z
            target = Vector3Add(lightPos, { 0.0f, 0.0f, 1.0f });
            up = { 0.0f, -1.0f, 0.0f };
            break;

        case 5: // -Z
        default:
            target = Vector3Add(lightPos, { 0.0f, 0.0f, -1.0f });
            up = { 0.0f, -1.0f, 0.0f };
            break;
    }

    Matrix view = MatrixLookAt(lightPos, target, up);

    Matrix projection = MatrixPerspective(
        90.0f * DEG2RAD,
        1.0f,
        POINT_SHADOW_NEAR,
        POINT_SHADOW_FAR
    );

    // Keep this order because your earlier debug showed this one behaves correctly.
    return MatrixMultiply(view, projection);
}

static void InitPointShadowMaps()
{
    for (int i = 0; i < 6; i++)
    {
        pointShadowMapData.fbo[i] = rlLoadFramebuffer();

        rlEnableFramebuffer(pointShadowMapData.fbo[i]);

        pointShadowMapData.depth[i].id = rlLoadTextureDepth(
            POINT_SHADOW_SIZE,
            POINT_SHADOW_SIZE,
            false
        );

        pointShadowMapData.depth[i].width = POINT_SHADOW_SIZE;
        pointShadowMapData.depth[i].height = POINT_SHADOW_SIZE;
        pointShadowMapData.depth[i].mipmaps = 1;
        pointShadowMapData.depth[i].format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;

        rlFramebufferAttach(
            pointShadowMapData.fbo[i],
            pointShadowMapData.depth[i].id,
            RL_ATTACHMENT_DEPTH,
            RL_ATTACHMENT_TEXTURE2D,
            0
        );

        if (!rlFramebufferComplete(pointShadowMapData.fbo[i]))
        {
            TraceLog(LOG_WARNING, "Point shadow framebuffer %i is not complete", i);
        }

        rlDisableFramebuffer();

        SetTextureFilter(pointShadowMapData.depth[i], TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(pointShadowMapData.depth[i], TEXTURE_WRAP_CLAMP);
    }
}

Shader GetShadowShader()
{
    return shadowShader;
}

bool HasShadowCastingPointLight()
{
    Vector3 pos = { 0.0f, 0.0f, 0.0f };
    int activeIndex = -1;

    return GetPrimaryShadowPointLight(&pos, &activeIndex);
}

int GetPointShadowFaceCount()
{
    return 6;
}

Camera3D GetPointShadowCamera(int face)
{
    Vector3 lightPos = { 0.0f, 0.0f, 0.0f };
    int activeIndex = -1;

    GetPrimaryShadowPointLight(&lightPos, &activeIndex);

    Camera3D cam = { 0 };

    cam.position = lightPos;
    cam.target = lightPos;
    cam.up = { 0.0f, -1.0f, 0.0f };
    cam.fovy = 90.0f;
    cam.projection = CAMERA_PERSPECTIVE;

    switch (face)
    {
        case 0:
            cam.target = Vector3Add(lightPos, { 1.0f, 0.0f, 0.0f });
            cam.up = { 0.0f, -1.0f, 0.0f };
            break;

        case 1:
            cam.target = Vector3Add(lightPos, { -1.0f, 0.0f, 0.0f });
            cam.up = { 0.0f, -1.0f, 0.0f };
            break;

        case 2:
            cam.target = Vector3Add(lightPos, { 0.0f, 1.0f, 0.0f });
            cam.up = { 0.0f, 0.0f, 1.0f };
            break;

        case 3:
            cam.target = Vector3Add(lightPos, { 0.0f, -1.0f, 0.0f });
            cam.up = { 0.0f, 0.0f, -1.0f };
            break;

        case 4:
            cam.target = Vector3Add(lightPos, { 0.0f, 0.0f, 1.0f });
            cam.up = { 0.0f, -1.0f, 0.0f };
            break;

        case 5:
        default:
            cam.target = Vector3Add(lightPos, { 0.0f, 0.0f, -1.0f });
            cam.up = { 0.0f, -1.0f, 0.0f };
            break;
    }

    return cam;
}

void BeginPointShadowMapFace(int face)
{
    if (face < 0 || face >= 6) return;

    rlDrawRenderBatchActive();

    rlEnableFramebuffer(pointShadowMapData.fbo[face]);

    rlViewport(0, 0, POINT_SHADOW_SIZE, POINT_SHADOW_SIZE);

    rlClearScreenBuffers();

    rlEnableDepthTest();
    rlEnableDepthMask();

    rlColorMask(false, false, false, false);

    Matrix lightSpaceMatrix = GetPointLightSpaceMatrix(face);

    SetShaderValueMatrix(
        shadowShader,
        pointLightSpaceMatrixLoc,
        lightSpaceMatrix
    );
}

void EndPointShadowMapFace()
{
    rlDrawRenderBatchActive();

    rlColorMask(true, true, true, true);

    rlDisableFramebuffer();

    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
}

void BindPointShadowMaps()
{
    for (int i = 0; i < 6; i++)
    {
        if (pointShadowMapData.depth[i].id == 0) continue;

        rlActiveTextureSlot(POINT_SHADOW_TEXTURE_SLOT_START + i);
        rlEnableTexture(pointShadowMapData.depth[i].id);
    }

    rlActiveTextureSlot(0);
}

void InitLighting()
{
    const char* vsPath = "shaders/lighting.vs";
    const char* fsPath = "shaders/lighting.fs";

    if (!FileExists(vsPath))
    {
        std::cerr << "Missing vertex shader: " << vsPath << std::endl;
    }

    if (!FileExists(fsPath))
    {
        std::cerr << "Missing fragment shader: " << fsPath << std::endl;
    }

    lightingShader = LoadShader(vsPath, fsPath);

    lightingShader.locs[SHADER_LOC_MATRIX_MVP] =
        GetShaderLocation(lightingShader, "mvp");

    lightingShader.locs[SHADER_LOC_MATRIX_MODEL] =
        GetShaderLocation(lightingShader, "matModel");

    lightingShader.locs[SHADER_LOC_MATRIX_NORMAL] =
        GetShaderLocation(lightingShader, "matNormal");

    lightingShader.locs[SHADER_LOC_MAP_ALBEDO] =
        GetShaderLocation(lightingShader, "albedoMap");

    lightingShader.locs[SHADER_LOC_MAP_NORMAL] =
        GetShaderLocation(lightingShader, "normalMap");

    lightingShader.locs[SHADER_LOC_MAP_METALNESS] =
        GetShaderLocation(lightingShader, "metallicMap");

    lightingShader.locs[SHADER_LOC_MAP_ROUGHNESS] =
        GetShaderLocation(lightingShader, "roughnessMap");

    lightingShader.locs[SHADER_LOC_MAP_OCCLUSION] =
        GetShaderLocation(lightingShader, "aoMap");

    lightingShader.locs[SHADER_LOC_MAP_HEIGHT] =
        GetShaderLocation(lightingShader, "heightMap");

    lightingShader.locs[SHADER_LOC_COLOR_DIFFUSE] =
        GetShaderLocation(lightingShader, "colDiffuse");

    viewPosLoc = GetShaderLocation(lightingShader, "viewPos");
    ambientColorLoc = GetShaderLocation(lightingShader, "ambientColor");

    heightScaleLoc = GetShaderLocation(lightingShader, "heightScale");
    useParallaxMappingLoc = GetShaderLocation(lightingShader, "useParallaxMapping");

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

    rayTracingEnabledLoc = GetShaderLocation(lightingShader, "rayTracingEnabled");

    rtCubeCountLoc = GetShaderLocation(lightingShader, "rtCubeCount");
    rtSphereCountLoc = GetShaderLocation(lightingShader, "rtSphereCount");
    rtCylinderCountLoc = GetShaderLocation(lightingShader, "rtCylinderCount");

    for (int i = 0; i < MAX_RAYTRACE_OBJECTS; i++)
    {
        rtCubePositionsLoc[i] = GetShaderLocation(lightingShader, TextFormat("rtCubePositions[%i]", i));
        rtCubeRotationsLoc[i] = GetShaderLocation(lightingShader, TextFormat("rtCubeRotations[%i]", i));
        rtCubeScalesLoc[i] = GetShaderLocation(lightingShader, TextFormat("rtCubeScales[%i]", i));

        rtSpherePositionsLoc[i] = GetShaderLocation(lightingShader, TextFormat("rtSpherePositions[%i]", i));
        rtSphereScalesLoc[i] = GetShaderLocation(lightingShader, TextFormat("rtSphereScales[%i]", i));

        rtCylinderPositionsLoc[i] = GetShaderLocation(lightingShader, TextFormat("rtCylinderPositions[%i]", i));
        rtCylinderRotationsLoc[i] = GetShaderLocation(lightingShader, TextFormat("rtCylinderRotations[%i]", i));
        rtCylinderScalesLoc[i] = GetShaderLocation(lightingShader, TextFormat("rtCylinderScales[%i]", i));
    }

    usePointLightShadowLoc =
        GetShaderLocation(lightingShader, "usePointLightShadow");

    shadowPointLightIndexLoc =
        GetShaderLocation(lightingShader, "shadowPointLightIndex");

    shadowPointLightPositionLoc =
        GetShaderLocation(lightingShader, "shadowPointLightPosition");

    shadowCameraMaxDistanceLoc =
        GetShaderLocation(lightingShader, "shadowCameraMaxDistance");

    shadowCameraFadeRangeLoc =
        GetShaderLocation(lightingShader, "shadowCameraFadeRange");

    pointShadowMapLoc[0] = GetShaderLocation(lightingShader, "pointShadowMap0");
    pointShadowMapLoc[1] = GetShaderLocation(lightingShader, "pointShadowMap1");
    pointShadowMapLoc[2] = GetShaderLocation(lightingShader, "pointShadowMap2");
    pointShadowMapLoc[3] = GetShaderLocation(lightingShader, "pointShadowMap3");
    pointShadowMapLoc[4] = GetShaderLocation(lightingShader, "pointShadowMap4");
    pointShadowMapLoc[5] = GetShaderLocation(lightingShader, "pointShadowMap5");

    for (int i = 0; i < 6; i++)
    {
        pointLightSpaceMatricesLoc[i] = GetShaderLocation(
            lightingShader,
            TextFormat("pointLightSpaceMatrices[%i]", i)
        );

        int textureSlot = POINT_SHADOW_TEXTURE_SLOT_START + i;

        SetShaderValue(
            lightingShader,
            pointShadowMapLoc[i],
            &textureSlot,
            SHADER_UNIFORM_INT
        );
    }

    const char* shadowVsPath = "shaders/shadowmap.vs";
    const char* shadowFsPath = "shaders/shadowmap.fs";

    if (!FileExists(shadowVsPath))
    {
        std::cerr << "Missing shadow vertex shader: " << shadowVsPath << std::endl;
    }

    if (!FileExists(shadowFsPath))
    {
        std::cerr << "Missing shadow fragment shader: " << shadowFsPath << std::endl;
    }

    shadowShader = LoadShader(shadowVsPath, shadowFsPath);

    shadowShader.locs[SHADER_LOC_MATRIX_MODEL] =
        GetShaderLocation(shadowShader, "matModel");

    pointLightSpaceMatrixLoc =
        GetShaderLocation(shadowShader, "pointLightSpaceMatrix");

    InitPointShadowMaps();
}

void ApplyLightingShader(Model& model)
{
    for (int i = 0; i < model.materialCount; i++)
    {
        model.materials[i].shader = lightingShader;
    }
}

void UpdateLighting(Camera3D camera)
{
    Vector3 viewPos = camera.position;

    // Low ambient so areas not reached by point lights are dark.
    Vector4 ambientColor = { 0.005f, 0.005f, 0.006f, 1.0f };

    int useParallaxMapping = 0;
    float heightScale = 0.0f;

    SetShaderValue(
        lightingShader,
        useParallaxMappingLoc,
        &useParallaxMapping,
        SHADER_UNIFORM_INT
    );

    SetShaderValue(
        lightingShader,
        heightScaleLoc,
        &heightScale,
        SHADER_UNIFORM_FLOAT
    );

    SetShaderValue(
        lightingShader,
        viewPosLoc,
        &viewPos,
        SHADER_UNIFORM_VEC3
    );

    SetShaderValue(
        lightingShader,
        ambientColorLoc,
        &ambientColor,
        SHADER_UNIFORM_VEC4
    );

    int activeLightCount = 0;

    if (!rayTracingEnabled)
    {
        // Legacy shadow-map mode: keep scene order so shader light 0 matches
        // the old primary shadow-map light.
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

            SetShaderValue(lightingShader, lightPositionsLoc[activeLightCount], &position, SHADER_UNIFORM_VEC3);
            SetShaderValue(lightingShader, lightColorsLoc[activeLightCount], &color, SHADER_UNIFORM_VEC4);
            SetShaderValue(lightingShader, lightIntensitiesLoc[activeLightCount], &intensity, SHADER_UNIFORM_FLOAT);
            SetShaderValue(lightingShader, lightRadiiLoc[activeLightCount], &radius, SHADER_UNIFORM_FLOAT);

            activeLightCount++;
        }
    }
    else
    {
        bool usedLights[MAX_SCENE_LIGHTS] = { false };

        // Ray-traced mode: upload nearest active point lights first. This lets
        // the editor store many lights while staying inside GLSL uniform limits.
        for (int slot = 0; slot < MAX_SHADER_LIGHTS; slot++)
        {
            int bestIndex = -1;
            float bestDistance = FLT_MAX;

            for (int i = 0; i < sceneLightCount; i++)
            {
                if (usedLights[i]) continue;
                if (!sceneLights[i].enabled) continue;
                if (sceneLights[i].type != SCENE_LIGHT_POINT) continue;

                float distanceToCamera = Vector3DistanceSqr(sceneLights[i].position, camera.position);

                if (distanceToCamera < bestDistance)
                {
                    bestDistance = distanceToCamera;
                    bestIndex = i;
                }
            }

            if (bestIndex < 0) break;

            usedLights[bestIndex] = true;

            Vector3 position = sceneLights[bestIndex].position;

            Vector4 color = {
                sceneLights[bestIndex].color.r / 255.0f,
                sceneLights[bestIndex].color.g / 255.0f,
                sceneLights[bestIndex].color.b / 255.0f,
                1.0f
            };

            float intensity = sceneLights[bestIndex].intensity;
            float radius = sceneLights[bestIndex].radius;

            SetShaderValue(lightingShader, lightPositionsLoc[activeLightCount], &position, SHADER_UNIFORM_VEC3);
            SetShaderValue(lightingShader, lightColorsLoc[activeLightCount], &color, SHADER_UNIFORM_VEC4);
            SetShaderValue(lightingShader, lightIntensitiesLoc[activeLightCount], &intensity, SHADER_UNIFORM_FLOAT);
            SetShaderValue(lightingShader, lightRadiiLoc[activeLightCount], &radius, SHADER_UNIFORM_FLOAT);

            activeLightCount++;
        }
    }

    SetShaderValue(
        lightingShader,
        lightCountLoc,
        &activeLightCount,
        SHADER_UNIFORM_INT
    );

    int rtEnabled = rayTracingEnabled ? 1 : 0;
    SetShaderValue(lightingShader, rayTracingEnabledLoc, &rtEnabled, SHADER_UNIFORM_INT);

    SetShaderValue(lightingShader, rtCubeCountLoc, &rtCubeCount, SHADER_UNIFORM_INT);
    SetShaderValue(lightingShader, rtSphereCountLoc, &rtSphereCount, SHADER_UNIFORM_INT);
    SetShaderValue(lightingShader, rtCylinderCountLoc, &rtCylinderCount, SHADER_UNIFORM_INT);

    for (int i = 0; i < rtCubeCount; i++)
    {
        SetShaderValue(lightingShader, rtCubePositionsLoc[i], &rtCubePositions[i], SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, rtCubeRotationsLoc[i], &rtCubeRotations[i], SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, rtCubeScalesLoc[i], &rtCubeScales[i], SHADER_UNIFORM_VEC3);
    }

    for (int i = 0; i < rtSphereCount; i++)
    {
        SetShaderValue(lightingShader, rtSpherePositionsLoc[i], &rtSpherePositions[i], SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, rtSphereScalesLoc[i], &rtSphereScales[i], SHADER_UNIFORM_VEC3);
    }

    for (int i = 0; i < rtCylinderCount; i++)
    {
        SetShaderValue(lightingShader, rtCylinderPositionsLoc[i], &rtCylinderPositions[i], SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, rtCylinderRotationsLoc[i], &rtCylinderRotations[i], SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, rtCylinderScalesLoc[i], &rtCylinderScales[i], SHADER_UNIFORM_VEC3);
    }

    Vector3 shadowLightPos = { 0.0f, 0.0f, 0.0f };
    int shadowActiveIndex = -1;
    int usePointLightShadow = 0;

    // Important: the legacy shadow-map system only supports ONE primary point light.
    // Do not enable it here, otherwise users see one-light-only shadows whenever
    // ray tracing is OFF. Ray tracing ON = multi-light shadows. Ray tracing OFF =
    // normal PBR lighting without shadows.
    (void)shadowLightPos;
    (void)shadowActiveIndex;
    usePointLightShadow = 0;

    SetShaderValue(
        lightingShader,
        usePointLightShadowLoc,
        &usePointLightShadow,
        SHADER_UNIFORM_INT
    );

    SetShaderValue(
        lightingShader,
        shadowPointLightIndexLoc,
        &shadowActiveIndex,
        SHADER_UNIFORM_INT
    );

    SetShaderValue(
        lightingShader,
        shadowPointLightPositionLoc,
        &shadowLightPos,
        SHADER_UNIFORM_VEC3
    );

    float maxShadowDistance = SHADOW_CAMERA_MAX_DISTANCE;
    float fadeRange = SHADOW_CAMERA_FADE_RANGE;

    SetShaderValue(
        lightingShader,
        shadowCameraMaxDistanceLoc,
        &maxShadowDistance,
        SHADER_UNIFORM_FLOAT
    );

    SetShaderValue(
        lightingShader,
        shadowCameraFadeRangeLoc,
        &fadeRange,
        SHADER_UNIFORM_FLOAT
    );

    if (usePointLightShadow == 1)
    {
        for (int i = 0; i < 6; i++)
        {
            Matrix m = GetPointLightSpaceMatrix(i);

            SetShaderValueMatrix(
                lightingShader,
                pointLightSpaceMatricesLoc[i],
                m
            );
        }
    }
}

void UnloadLighting()
{
    if (lightingShader.id != 0)
    {
        UnloadShader(lightingShader);
        lightingShader = { 0 };
    }

    if (shadowShader.id != 0)
    {
        UnloadShader(shadowShader);
        shadowShader = { 0 };
    }

    for (int i = 0; i < 6; i++)
    {
        if (pointShadowMapData.depth[i].id != 0)
        {
            rlUnloadTexture(pointShadowMapData.depth[i].id);
            pointShadowMapData.depth[i] = { 0 };
        }

        if (pointShadowMapData.fbo[i] != 0)
        {
            rlUnloadFramebuffer(pointShadowMapData.fbo[i]);
            pointShadowMapData.fbo[i] = 0;
        }
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
    light.intensity = 70.0f;
    light.radius = 70.0f;

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

    hitPosition.y += 3.0f;

    return CreatePointLight(hitPosition) != -1;
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

    return CreatePointLight(position) != -1;
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

        DrawSphere(pos, 0.18f, color);
        DrawSphereWires(pos, 0.22f, 8, 8, WHITE);
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