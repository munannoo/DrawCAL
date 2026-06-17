#ifndef object_h
#define object_h

#include "raylib.h"

void initModels();
void DrawSceneForShadowMap();
void cube(const Vector3 pos,Color color=GRAY);
void sphere(const Vector3 pos,Color color=GRAY);
void cylinder(const Vector3 pos,Color color=GRAY);
void frameCube();
void frameSphere();
void frameCylinder();
void Unload();
void leftclick(Ray ray);
bool updateObjectTransformGizmo(Camera3D camera);
void drawObjectTransformGizmo();
void lightSphere(const Vector3 pos, Color color);
void deleteobj();
#endif // object_h