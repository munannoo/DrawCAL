#include "lighting.h"

#include "raylib.h"
#include "raymath.h"

#include <cmath>
#include <float.h>
#include <iostream>
#include <algorithm>

std::vector<std::unique_ptr<Light>> lights;
std::vector<Light*> selectedLights;
Light* activeLight = nullptr;

// Light implementation

unsigned int Light::nextId = 0;

Light::Light(Vector3 position, Vector3 target, Color color, R3D_LightType type)
    : position(position), target(target), color(color)
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

Vector3 Light::getPosition() const
{
    return position;
}

void Light::setPosition(Vector3 newPosition)
{
    position = newPosition;
    R3D_LightLookAt(light, position, target);
}

Vector3 Light::getTarget() const
{
    return target;
}

void Light::setTarget(Vector3 newTarget)
{
    target = newTarget;
    R3D_LightLookAt(light, position, target);
}

Color Light::getColor() const
{
    return color;
}

float Light::getPickRadius() const
{
    return 0.35f;
}

// --- Selection, mirrors object.cpp's shape selection ---

static void updateSelectedLights()
{
    selectedLights.clear();

    for (const auto& lightPtr : lights)
    {
        if (lightPtr && lightPtr->getSelected())
        {
            selectedLights.push_back(lightPtr.get());
        }
    }
}

void selectLightByRay(Ray ray)
{
    Light* closestLight = nullptr;
    float closestDist = FLT_MAX;

    for (auto& lightPtr : lights)
    {
        RayCollision collision = GetRayCollisionSphere(ray, lightPtr->getPosition(), lightPtr->getPickRadius());

        if (collision.hit && collision.distance < closestDist) {
            closestDist = collision.distance;
            closestLight = lightPtr.get();
        }
    }

    const bool additiveSelection = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    selectLights(closestLight, additiveSelection);
}

void selectLights(Light* closestLight, bool additiveSelection)
{
    if (!additiveSelection)
    {
        for (auto& lightPtr : lights) lightPtr->setSelected(false);
        activeLight = nullptr;
    }

    if (closestLight != nullptr)
    {
        if (additiveSelection)
        {
            closestLight->setSelected(!closestLight->getSelected());
            if (closestLight->getSelected()) activeLight = closestLight;
            else if (closestLight == activeLight) activeLight = nullptr;
        }
        else
        {
            closestLight->setSelected(true);
            activeLight = closestLight;
        }

        TraceLog(
            LOG_INFO,
            "Clicked light ID: %u",
            closestLight->getId()
        );
    }

    updateSelectedLights();
}

void deleteLights()
{
    lights.erase(
        std::remove_if(
            lights.begin(),
            lights.end(),
            [](const std::unique_ptr<Light>& l) { return l->getSelected(); }
        ),
        lights.end()
    );
    selectedLights.clear();
    activeLight = nullptr;
}

void initialiseEnvironment() {

    R3D_Environment* env = R3D_GetEnvironment();

    env->background.color = Color{ 45, 48, 52, 255 };

    env->ambient.color = Color{ 20, 20, 20, 255 };
    env->ambient.energy = 0.3f;

    env->ssao.enabled = true;
    env->ssao.intensity = 0.7f;
    env->ssao.radius = 0.5f;
    env->ssao.bias = 0.03f;

    env->ssil.enabled = true;

    env->ssgi.enabled = false;
    env->ssr.enabled = false;

    R3D_SetEnvironment(env);
}