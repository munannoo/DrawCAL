#ifndef object_h
#define object_h

#include "raylib.h"
#include "features/manipulation/transform.h"
#include <float.h>
#include "raymath.h"
#include "rlgl.h"
#include "features/shadings/textures.h"
#include "features/shadings/lighting.h"
#include "data/save_load/saveNload.h"
#include <iostream>
#include <vector>

void initModels();
void DrawSceneForShadowMap();
void UploadSceneToRayTracer();
void SyncObjectLightsToScene();
void cube(const Vector3 pos, Color color = GRAY);
void sphere(const Vector3 pos, Color color = GRAY);
void cylinder(const Vector3 pos, Color color = GRAY);
void renderCube();
void renderSphere();
void renderCylinder();

// Unload the models, required because the objects are defined static, and 
void unloadModels();

void leftclick(Ray ray);
bool updateObjectTransformGizmo(Camera3D camera);
// obsolete ig, but idk
// void drawObjectTransformGizmo();
//// Query selected objects
//// Returns true and fills out when a selected object is found (first encountered)
//// outType: 1=cube,2=sphere,3=cylinder
bool getFirstSelected(ObjectInstance* out, int* outType, int* outIndex);
// Returns the total number of selected objects
int getTotalSelectedCount();
void drawObjectTransformGizmo(Camera3D camera);
void lightSphere(const Vector3 pos, Color color);
void deleteobj();

//load and save functions, defined in object.cpp but main logic is present in saveNload.cpp
void load();
void save();
#endif // object_h
