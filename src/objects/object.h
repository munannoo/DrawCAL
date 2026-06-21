#ifndef object_h
#define object_h

#include "raylib.h"
#include "features/manipulation/transform.h"
void initModels();
void cube(const Vector3 pos);
void sphere(const Vector3 pos);
void cylinder(const Vector3 pos);
void frameCube();
void frameSphere();
void frameCylinder();
void Unload();
void leftclick(Ray ray);
bool updateObjectTransformGizmo(Camera3D camera);
void drawObjectTransformGizmo();
// Query selected objects
// Returns true and fills out when a selected object is found (first encountered)
// outType: 1=cube,2=sphere,3=cylinder
bool getFirstSelected(ObjectInstance* out, int* outType, int* outIndex);
// Returns the total number of selected objects
int getTotalSelectedCount();
#endif // object_h