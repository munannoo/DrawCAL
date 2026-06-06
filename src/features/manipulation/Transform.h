#ifndef TRANSFORM_H
#define TRANSFORM_H
#include<raylib.h>

typedef struct ObjectInstance {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Color color;
    bool isSelected;
} ObjectInstance;
typedef enum GizmoState {
    GIZMO_NONE = 0,

    GIZMO_MOVE_X,
    GIZMO_MOVE_Y,
    GIZMO_MOVE_Z,

    GIZMO_SCALE_X,
    GIZMO_SCALE_Y,
    GIZMO_SCALE_Z,

    GIZMO_ROTATE_X,
    GIZMO_ROTATE_Y,
    GIZMO_ROTATE_Z
} GizmoState;

bool UpdateTransformGizmo(
    Camera3D camera,
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
);
void DrawTransformGizmo(
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
);

BoundingBox GetTransformedBounds(BoundingBox baseBox, Vector3 pos, Vector3 scale);
void InitTransformGizmo();
void UnloadTransformGizmo();
#endif // TRANSFORM_H