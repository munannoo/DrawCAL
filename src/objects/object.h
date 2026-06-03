#ifndef object_h
#define object_h

#include "raylib.h"

void initModels();
void cube(const Vector3 pos);
void sphere(const Vector3 pos);
void cylinder(const Vector3 pos);
void frameCube();
void frameSphere();
void frameCylinder();
void Unload();
#endif // object_h