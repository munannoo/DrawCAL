#ifndef LIGHTING_H
#define LIGHTING_H
#include "raylib.h"
#include <r3d.h>
#include <vector>
#include <memory>
#include "raygui.h"

class Light
{
private:
    R3D_Light light;
    unsigned int id;
    bool selected;
    Vector3 position;
    Vector3 target;
    Color color;

    static unsigned int nextId;

public:
    Light(Vector3 position = { 10,10,0 }, Vector3 target = { 0,0,0 }, Color color = RED, R3D_LightType type = R3D_LIGHT_SPOT);
    ~Light();

    R3D_Light getLight() const;

    unsigned int getId() const;

    bool getSelected() const;
    void setSelected(bool value);

    Vector3 getPosition() const;
    void setPosition(Vector3 newPosition);

    Vector3 getTarget() const;
    void setTarget(Vector3 newTarget);

    Color getColor() const;

    // World-space radius used both for drawing a selection indicator
    // and for ray-based picking, since lights have no mesh/AABB.
    float getPickRadius() const;
};

extern std::vector<std::unique_ptr<Light>> lights;
extern std::vector<Light*> selectedLights;
extern Light* activeLight;

void initialiseEnvironment();

// Mirrors selectObjectByRay/selectObjects in object.cpp, but for lights only.
void selectLightByRay(Ray ray);
void selectLights(Light* closestLight, bool additiveSelection);
void deleteLights();

#endif