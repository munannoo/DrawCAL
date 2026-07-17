#ifndef OBJECT_H
#define OBJECT_H

#include "raylib.h"
#include "features/manipulation/Transform.h"

void initModels();
void SyncObjectLightsToScene();

void cube(const Vector3 pos, Color color = GRAY);
void sphere(const Vector3 pos, Color color = GRAY);
void cylinder(const Vector3 pos, Color color = GRAY);
void lightSphere(const Vector3 pos, Color color);

void renderCube();
void renderSphere();
void renderCylinder();

void unloadModels();
void clear();
void deleteobj();

void leftclick(Ray ray);
bool updateObjectTransformGizmo(Camera3D camera);
void drawObjectTransformGizmo(Camera3D camera);

int getTotalSelectedCount();
ObjectInstance* getFirstSelectedMutable(int* outType = nullptr,
                                        int* outIndex = nullptr);
int getObjectCount(int objectType);
ObjectInstance* getObjectMutable(int objectType, int objectIndex);
bool selectObject(int objectType, int objectIndex, bool additive = false);
void deselectAllObjects();

void load();
void save();

#endif
