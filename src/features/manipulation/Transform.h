#ifndef TRANSFORM_H
#define TRANSFORM_H
#include<raylib.h>

typedef struct ObjectInstance {
    Vector3 position;
    Vector3 rotationAxis;
    float rotationAngle;
    Vector3 scale;
    Color color;
    bool isSelected;
} ObjectInstance;

BoundingBox GetTransformedBounds(BoundingBox baseBox, Vector3 pos, Vector3 scale);
#endif // TRANSFORM_H