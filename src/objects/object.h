#ifndef object_h
#define object_h

#include "raylib.h"
#include "features/manipulation/transform.h"
void initModels();
void DrawSceneForShadowMap();
void UploadSceneToRayTracer();
void SyncObjectLightsToScene();
void cube(const Vector3 pos, Color color = GRAY);
void sphere(const Vector3 pos, Color color = GRAY);
void cylinder(const Vector3 pos, Color color = GRAY);
void frameCube();
void frameSphere();
void frameCylinder();
void Unload();
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
#endif // object_h
