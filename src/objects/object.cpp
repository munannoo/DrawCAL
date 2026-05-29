#include "object.h"
void Cube(const Vector3 pos){
    Vector3 cubePosition = pos;
    DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
}
void Sphere(const Vector3 pos){
    Vector3 spherePosition = pos;
    DrawSphere(spherePosition, 2.0f, RED);
}
void Cylinder(const Vector3 pos){
    Vector3 cylinderPosition = pos;
    DrawCylinder(cylinderPosition, 1.0f, 1.0f, 2.0f, 16, RED);
}