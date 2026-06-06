#include <raylib.h>
#include "Transform.h"
#include "raymath.h"
#include "rlgl.h"
#include "math.h"
static Model ringModel;
static bool ringModelLoaded = false;
static GizmoState currentGizmoState = GIZMO_NONE;
static Vector3 gizmoCenter = { 0.0f, 0.0f, 0.0f };
void InitTransformGizmo()
{
    if (ringModelLoaded) return;

    Mesh torusMesh = GenMeshTorus(1.0f, 0.12f, 32, 64);
    ringModel = LoadModelFromMesh(torusMesh);

    ringModelLoaded = true;
}
void UnloadTransformGizmo()
{
    if (ringModelLoaded) {
        UnloadModel(ringModel);
        ringModelLoaded = false;
    }
}
BoundingBox GetTransformedBounds(BoundingBox baseBox, Vector3 pos, Vector3 scale) {
    BoundingBox transformed;
    
    transformed.min = { 
        baseBox.min.x * scale.x, 
        baseBox.min.y * scale.y, 
        baseBox.min.z * scale.z 
    };
    transformed.max = { 
        baseBox.max.x * scale.x, 
        baseBox.max.y * scale.y, 
        baseBox.max.z * scale.z 
    };
    
    // Shift boundary coordinates to world space position
    transformed.min = Vector3Add(transformed.min, pos);
    transformed.max = Vector3Add(transformed.max, pos);
    return transformed;
}

static bool HasSelectedObject(
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
) {
    for (int i = 0; i < cubeCount; i++) {
        if (cubes[i].isSelected) return true;
    }
    for (int i = 0; i < sphereCount; i++) {
        if (spheres[i].isSelected) return true;
    }
    for (int i = 0; i < cylinderCount; i++) {
        if (cylinders[i].isSelected) return true;
    }
    return false;
}

static Vector3 GetGizmoCenter(
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
) {
    Vector3 center = { 0.0f, 0.0f, 0.0f };
    int count = 0;

    for (int i = 0; i < cubeCount; i++) {
        if (cubes[i].isSelected) {
            center = Vector3Add(center, cubes[i].position);
            count++;
        }
    }
    for (int i = 0; i < sphereCount; i++) {
        if (spheres[i].isSelected) {
            center = Vector3Add(center, spheres[i].position);
            count++;
        }
    }
    for (int i = 0; i < cylinderCount; i++) {
        if (cylinders[i].isSelected) {
            center = Vector3Add(center, cylinders[i].position);
            count++;
        }
    }

    if (count > 0) {
        center = Vector3Scale(center, 1.0f / count);
    }

    return center;
}
static void MoveSelectedObjects(
    Vector3 delta,
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
) {
    for (int i = 0; i < cubeCount; i++) {
        if (cubes[i].isSelected) {
            cubes[i].position = Vector3Add(cubes[i].position, delta);
        }
    }
    for (int i = 0; i < sphereCount; i++) {
        if (spheres[i].isSelected) {
            spheres[i].position = Vector3Add(spheres[i].position, delta);
        }
    }
    for (int i = 0; i < cylinderCount; i++) {
        if (cylinders[i].isSelected) {
            cylinders[i].position = Vector3Add(cylinders[i].position, delta);
        }
    }
}

static void ScaleSelectedObjects(
    Vector3 scaleDelta,
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
) {
    for (int i = 0; i < cubeCount; i++) {
        if (cubes[i].isSelected) {
            cubes[i].scale = Vector3Add(cubes[i].scale, scaleDelta);

            if (cubes[i].scale.x < 0.1f) cubes[i].scale.x = 0.1f;
            if (cubes[i].scale.y < 0.1f) cubes[i].scale.y = 0.1f;
            if (cubes[i].scale.z < 0.1f) cubes[i].scale.z = 0.1f;
        }
    }

    for (int i = 0; i < sphereCount; i++) {
        if (spheres[i].isSelected) {
            spheres[i].scale = Vector3Add(spheres[i].scale, scaleDelta);

            if (spheres[i].scale.x < 0.1f) spheres[i].scale.x = 0.1f;
            if (spheres[i].scale.y < 0.1f) spheres[i].scale.y = 0.1f;
            if (spheres[i].scale.z < 0.1f) spheres[i].scale.z = 0.1f;
        }
    }

    for (int i = 0; i < cylinderCount; i++) {
        if (cylinders[i].isSelected) {
            cylinders[i].scale = Vector3Add(cylinders[i].scale, scaleDelta);

            if (cylinders[i].scale.x < 0.1f) cylinders[i].scale.x = 0.1f;
            if (cylinders[i].scale.y < 0.1f) cylinders[i].scale.y = 0.1f;
            if (cylinders[i].scale.z < 0.1f) cylinders[i].scale.z = 0.1f;
        }
    }
}
static void RotateSelectedObjectsAxis(
    Vector3 rotationDelta,
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
) {
    for (int i = 0; i < cubeCount; i++) {
        if (cubes[i].isSelected) {
            cubes[i].rotation = Vector3Add(cubes[i].rotation, rotationDelta);
        }
    }
    for (int i = 0; i < sphereCount; i++) {
        if (spheres[i].isSelected) {
            spheres[i].rotation = Vector3Add(spheres[i].rotation, rotationDelta);
        }
    }
    for (int i = 0; i < cylinderCount; i++) {
        if (cylinders[i].isSelected) {
            cylinders[i].rotation = Vector3Add(cylinders[i].rotation, rotationDelta);
        }
    }
}

static bool checkGizmoClick(Ray ray, Vector3 center, float gizmoSize) {
    float objectRadius = gizmoSize * 1.3f;

    if (objectRadius < 1.3f) {
        objectRadius = 1.3f;
    }

    if (objectRadius > 6.0f) {
        objectRadius = 6.0f;
    }
    float moveStart = 1.0f * gizmoSize;
    float moveEnd   = 2.6f * gizmoSize;

    float scaleStart = 2.8f * gizmoSize;
    float scaleEnd   = 3.35f * gizmoSize;

    float axisThickness = 0.18f * gizmoSize;
    float scaleThickness = 0.35f * gizmoSize;

    BoundingBox moveXBox = {
        { center.x + moveStart, center.y - axisThickness, center.z - axisThickness },
        { center.x + moveEnd,   center.y + axisThickness, center.z + axisThickness }
    };

    BoundingBox moveYBox = {
        { center.x - axisThickness, center.y + moveStart, center.z - axisThickness },
        { center.x + axisThickness, center.y + moveEnd,   center.z + axisThickness }
    };

    BoundingBox moveZBox = {
        { center.x - axisThickness, center.y - axisThickness, center.z + moveStart },
        { center.x + axisThickness, center.y + axisThickness, center.z + moveEnd }
    };

    if (GetRayCollisionBox(ray, moveXBox).hit) {
        currentGizmoState = GIZMO_MOVE_X;
        return true;
    }

    if (GetRayCollisionBox(ray, moveYBox).hit) {
        currentGizmoState = GIZMO_MOVE_Y;
        return true;
    }

    if (GetRayCollisionBox(ray, moveZBox).hit) {
        currentGizmoState = GIZMO_MOVE_Z;
        return true;
    }

    BoundingBox scaleXBox = {
        { center.x + scaleStart, center.y - scaleThickness, center.z - scaleThickness },
        { center.x + scaleEnd,   center.y + scaleThickness, center.z + scaleThickness }
    };

    BoundingBox scaleYBox = {
        { center.x - scaleThickness, center.y + scaleStart, center.z - scaleThickness },
        { center.x + scaleThickness, center.y + scaleEnd,   center.z + scaleThickness }
    };

    BoundingBox scaleZBox = {
        { center.x - scaleThickness, center.y - scaleThickness, center.z + scaleStart },
        { center.x + scaleThickness, center.y + scaleThickness, center.z + scaleEnd }
    };

    if (GetRayCollisionBox(ray, scaleXBox).hit) {
        currentGizmoState = GIZMO_SCALE_X;
        return true;
    }

    if (GetRayCollisionBox(ray, scaleYBox).hit) {
        currentGizmoState = GIZMO_SCALE_Y;
        return true;
    }

    if (GetRayCollisionBox(ray, scaleZBox).hit) {
        currentGizmoState = GIZMO_SCALE_Z;
        return true;
    }

    // Bigger ring click area
    float outerRadius = objectRadius + 1.5f;
    float innerRadius = objectRadius + 0.5f;

    RayCollision outerRing = GetRayCollisionSphere(ray, center, outerRadius);
    RayCollision innerRing = GetRayCollisionSphere(ray, center, innerRadius);

    if (outerRing.hit && !innerRing.hit) {
        if (IsKeyDown(KEY_X)) {
            currentGizmoState = GIZMO_ROTATE_X;
        }
        else if (IsKeyDown(KEY_Z)) {
            currentGizmoState = GIZMO_ROTATE_Z;
        }
        else {
            currentGizmoState = GIZMO_ROTATE_Y;
        }

        return true;
    }

    return false;
}
static float GetSelectedMaxScale(
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
);
bool UpdateTransformGizmo(
    Camera3D camera,
    ObjectInstance* cubes, 
    int cubeCount,
    ObjectInstance* spheres, 
    int sphereCount,
    ObjectInstance* cylinders, 
    int cylinderCount
) {
    if (!HasSelectedObject(cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount)) {
        currentGizmoState = GIZMO_NONE;
        return false;
    }

    gizmoCenter = GetGizmoCenter(cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount);

    Vector2 mouseDelta = GetMouseDelta();
    Ray ray = GetMouseRay(GetMousePosition(), camera);

    bool clickedGizmo = false;
    float selectedMaxScale = GetSelectedMaxScale(
        cubes, cubeCount,
        spheres, sphereCount,
        cylinders, cylinderCount
    );

    float gizmoSize = selectedMaxScale;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        clickedGizmo = checkGizmoClick(ray, gizmoCenter, gizmoSize);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && currentGizmoState != GIZMO_NONE) {
        float moveSensitivity = 0.03f;
        float scaleSensitivity = 0.01f;
        float rotationSensitivity = 0.5f;

        switch (currentGizmoState) {
            case GIZMO_MOVE_X:
                MoveSelectedObjects(
                    { mouseDelta.x * moveSensitivity, 0.0f, 0.0f },
                    cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount
                );
                break;

            case GIZMO_MOVE_Y:
                MoveSelectedObjects(
                    { 0.0f, -mouseDelta.y * moveSensitivity, 0.0f },
                    cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount
                );
                break;

            case GIZMO_MOVE_Z:
                MoveSelectedObjects(
                    { 0.0f, 0.0f, -mouseDelta.x * moveSensitivity },
                    cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount
                );
                break;

            case GIZMO_SCALE_X:
                ScaleSelectedObjects(
                    { mouseDelta.x * scaleSensitivity, 0.0f, 0.0f },
                    cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount
                );
                break;

            case GIZMO_SCALE_Y:
                ScaleSelectedObjects(
                    { 0.0f, -mouseDelta.y * scaleSensitivity, 0.0f },
                    cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount
                );
                break;

            case GIZMO_SCALE_Z:
                ScaleSelectedObjects(
                    { 0.0f, 0.0f, -mouseDelta.x * scaleSensitivity },
                    cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount
                );
                break;

            case GIZMO_ROTATE_X:
                RotateSelectedObjectsAxis(
                    { mouseDelta.x * rotationSensitivity, 0.0f, 0.0f },
                    cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount
                );
                break;

            case GIZMO_ROTATE_Y:
                RotateSelectedObjectsAxis(
                    { 0.0f, mouseDelta.x * rotationSensitivity, 0.0f },
                    cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount
                );
                break;

            case GIZMO_ROTATE_Z:
                RotateSelectedObjectsAxis(
                    { 0.0f, 0.0f, mouseDelta.x * rotationSensitivity },
                    cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount
                );
                break;

            default:
                break;
        }

        return true;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        currentGizmoState = GIZMO_NONE;
    }

    return clickedGizmo;
}

void DrawTransformGizmo(
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
) {
    if (!HasSelectedObject(cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount)) {
        return;
    }
    Vector3 center = GetGizmoCenter(cubes, cubeCount, spheres, sphereCount, cylinders, cylinderCount);
    float selectedMaxScale = GetSelectedMaxScale(
        cubes, cubeCount,
        spheres, sphereCount,
        cylinders, cylinderCount
    );
    float gizmoSize = 1.0f + selectedMaxScale * 0.6f;
    
    float arrowLength = 2.0f * gizmoSize;
    float arrowTipEnd = 2.45f * gizmoSize;
    float scaleHandleDistance = 3.0f * gizmoSize;

    float axisRadius = 0.06f * gizmoSize;
    float coneRadius = 0.22f * gizmoSize;
    float cubeSize = 0.45f * gizmoSize;

    float objectRadius = selectedMaxScale * 1.3f;
    float ringXRadius = objectRadius + 0.8f;
    float ringYRadius = objectRadius + 1.0f;
    float ringZRadius = objectRadius + 1.2f;

    
    //Move X
    DrawCylinderEx(
        center,
        Vector3Add(center, { arrowLength, 0.0f, 0.0f }),
        axisRadius,
        axisRadius,
        12,
        RED
    );

    DrawCylinderEx(
        Vector3Add(center, { arrowLength, 0.0f, 0.0f }),
        Vector3Add(center, { arrowTipEnd, 0.0f, 0.0f }),
        coneRadius,
        0.0f,
        12,
        RED
    );

    DrawCylinderEx(
        Vector3Add(center, { arrowLength, 0.0f, 0.0f }),
        Vector3Add(center, { arrowTipEnd, 0.0f, 0.0f }),
        0.18f,
        0.0f,
        12,
        RED
    );

    //Move Y
    DrawCylinderEx(
        center,
        Vector3Add(center, { 0.0f, arrowLength, 0.0f }),
        0.05f,
        0.05f,
        12,
        GREEN
    );

    DrawCylinderEx(
        Vector3Add(center, { 0.0f, arrowLength, 0.0f }),
        Vector3Add(center, { 0.0f, arrowTipEnd, 0.0f }),
        0.18f,
        0.0f,
        12,
        GREEN
    );

    // Move Z
    DrawCylinderEx(
        center,
        Vector3Add(center, { 0.0f, 0.0f, arrowLength }),
        0.05f,
        0.05f,
        12,
        BLUE
    );

    DrawCylinderEx(
        Vector3Add(center, { 0.0f, 0.0f, arrowLength }),
        Vector3Add(center, { 0.0f, 0.0f, arrowTipEnd }),
        0.18f,
        0.0f,
        12,
        BLUE
    );
    DrawCube(
        Vector3Add(center, { scaleHandleDistance, 0.0f, 0.0f }),
        cubeSize,
        cubeSize,
        cubeSize,
        RED
    );

    DrawCube(
        Vector3Add(center, { 0.0f, scaleHandleDistance, 0.0f }),
        cubeSize,
        cubeSize,
        cubeSize,
        GREEN
    );

    DrawCube(
        Vector3Add(center, { 0.0f, 0.0f, scaleHandleDistance }),
        cubeSize,
        cubeSize,
        cubeSize,
        BLUE
    );
    
    rlDisableDepthTest();

    if (ringModelLoaded) {
    DrawModelEx(
        ringModel,
        center,
        { 0.0f, 1.0f, 0.0f },
        90.0f,
        { ringXRadius, ringXRadius, ringXRadius },
        RED
    );

    DrawModelEx(
        ringModel,
        center,
        { 1.0f, 0.0f, 0.0f },
        90.0f,
        { ringYRadius, ringYRadius, ringYRadius },
        GREEN
    );

    DrawModelEx(
        ringModel,
        center,
        { 0.0f, 0.0f, 1.0f },
        0.0f,
        { ringZRadius, ringZRadius, ringZRadius },
        BLUE
    );
    }

    rlEnableDepthTest();

}
static float GetSelectedMaxScale(
    ObjectInstance* cubes, int cubeCount,
    ObjectInstance* spheres, int sphereCount,
    ObjectInstance* cylinders, int cylinderCount
) {
    float maxScale = 1.0f;

    for (int i = 0; i < cubeCount; i++) {
        if (cubes[i].isSelected) {
            maxScale = fmaxf(maxScale, cubes[i].scale.x);
            maxScale = fmaxf(maxScale, cubes[i].scale.y);
            maxScale = fmaxf(maxScale, cubes[i].scale.z);
        }
    }

    for (int i = 0; i < sphereCount; i++) {
        if (spheres[i].isSelected) {
            maxScale = fmaxf(maxScale, spheres[i].scale.x);
            maxScale = fmaxf(maxScale, spheres[i].scale.y);
            maxScale = fmaxf(maxScale, spheres[i].scale.z);
        }
    }

    for (int i = 0; i < cylinderCount; i++) {
        if (cylinders[i].isSelected) {
            maxScale = fmaxf(maxScale, cylinders[i].scale.x);
            maxScale = fmaxf(maxScale, cylinders[i].scale.y);
            maxScale = fmaxf(maxScale, cylinders[i].scale.z);
        }
    }

    return maxScale;
}