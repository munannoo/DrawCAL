#ifndef TRANSFORM_H
#define TRANSFORM_H
#include<raylib.h>

typedef struct ObjectInstance {
    Vector3 position;
    Vector3 rotationAxis;
    float rotationAngle;
    Vector3 scale;
    Color color;
} ObjectInstance;

#endif // TRANSFORM_H