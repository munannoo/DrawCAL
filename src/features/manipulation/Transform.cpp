#include <raylib.h>
#include "Transform.h"
#include "raymath.h"
#include "rlgl.h"
#include "math.h"
static Model ringModel;
static bool ringModelLoaded = false;
static GizmoState currentGizmoState = GIZMO_NONE;
static Vector3 gizmoCenter = { 0.0f, 0.0f, 0.0f };

typedef struct GizmoDimensions {
    float size;
    float objectRadius;
    float arrowStart;
    float arrowLength;
    float arrowTipEnd;
    float scaleHandleDistance;
    float axisRadius;
    float axisPickRadius;
    float coneRadius;
    float cubeSize;
    float ringOuterX;
    float ringOuterY;
    float ringOuterZ;
    float ringPickPixels;
} GizmoDimensions;

static float ClampFloat(float value, float minValue, float maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

static GizmoDimensions GetGizmoDimensions(float selectedMaxScale)
{
    GizmoDimensions dims = { 0 };
    dims.size = ClampFloat(1.0f + (selectedMaxScale - 1.0f) * 0.25f, 0.9f, 6.0f);
    dims.objectRadius = fmaxf(selectedMaxScale * 1.25f, 1.1f);
    // Rotation rings stay closer to object
    dims.ringOuterX = dims.objectRadius + 0.55f * dims.size;
    dims.ringOuterY = dims.objectRadius + 0.78f * dims.size;
    dims.ringOuterZ = dims.objectRadius + 1.01f * dims.size;
    // Move arrows start AFTER the rotation rings
    float outerRing = dims.ringOuterZ;
    dims.arrowStart = outerRing + 0.35f * dims.size;
    dims.arrowLength = dims.arrowStart + 1.45f * dims.size;
    dims.arrowTipEnd = dims.arrowLength + 0.45f * dims.size;
    // Scale cubes move further too
    dims.scaleHandleDistance = dims.arrowTipEnd + 0.55f * dims.size;
    dims.axisRadius = 0.06f * dims.size;
    dims.axisPickRadius = 0.20f * dims.size;
    dims.coneRadius = 0.23f * dims.size;
    dims.cubeSize = 0.42f * dims.size;

    dims.ringPickPixels = 11.0f;

    return dims;
}

void InitTransformGizmo()
{
    if (ringModelLoaded) return;

    Mesh torusMesh = GenMeshTorus(0.8f, 4.0f, 16, 32);
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

static float DistancePointToSegment(Vector2 point, Vector2 start, Vector2 end)
{
    Vector2 segment = Vector2Subtract(end, start);
    float segmentLengthSq = Vector2DotProduct(segment, segment);

    if (segmentLengthSq <= 0.0001f) {
        return Vector2Distance(point, start);
    }

    float t = Vector2DotProduct(Vector2Subtract(point, start), segment) / segmentLengthSq;
    t = ClampFloat(t, 0.0f, 1.0f);

    Vector2 closest = Vector2Add(start, Vector2Scale(segment, t));
    return Vector2Distance(point, closest);
}

static float GetRingScreenDistance(
    Camera3D camera,
    Vector2 mouse,
    Vector3 center,
    Vector3 axisA,
    Vector3 axisB,
    float radius
) {
    float closestDistance = 1000000.0f;
    const int segments = 96;

    for (int i = 0; i < segments; i++) {
        float angleA = (2.0f * PI * i) / segments;
        float angleB = (2.0f * PI * (i + 1)) / segments;

        Vector3 worldA = Vector3Add(
            center,
            Vector3Add(
                Vector3Scale(axisA, cosf(angleA) * radius),
                Vector3Scale(axisB, sinf(angleA) * radius)
            )
        );
        Vector3 worldB = Vector3Add(
            center,
            Vector3Add(
                Vector3Scale(axisA, cosf(angleB) * radius),
                Vector3Scale(axisB, sinf(angleB) * radius)
            )
        );

        Vector2 screenA = GetWorldToScreen(worldA, camera);
        Vector2 screenB = GetWorldToScreen(worldB, camera);
        float distance = DistancePointToSegment(mouse, screenA, screenB);

        if (distance < closestDistance) {
            closestDistance = distance;
        }
    }

    return closestDistance;
}

static GizmoState GetClickedRing(Camera3D camera, Vector2 mouse, Vector3 center, GizmoDimensions dims)
{
    float closestRingDistance = dims.ringPickPixels;
    GizmoState closestRing = GIZMO_NONE;
    float ringDistance = GetRingScreenDistance(
        camera,
        mouse,
        center,
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
        dims.ringOuterX
    );

    if (ringDistance < closestRingDistance) {
        closestRingDistance = ringDistance;
        closestRing = GIZMO_ROTATE_X;
    }

    ringDistance = GetRingScreenDistance(
        camera,
        mouse,
        center,
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
        dims.ringOuterY
    );

    if (ringDistance < closestRingDistance) {
        closestRingDistance = ringDistance;
        closestRing = GIZMO_ROTATE_Y;
    }

    ringDistance = GetRingScreenDistance(
        camera,
        mouse,
        center,
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        dims.ringOuterZ
    );

    if (ringDistance < closestRingDistance) {
        closestRingDistance = ringDistance;
        closestRing = GIZMO_ROTATE_Z;
    }

    return closestRing;
}

static bool checkGizmoClick(Ray ray, Camera3D camera, Vector2 mouse, Vector3 center, GizmoDimensions dims) {
    GizmoState clickedRing = GetClickedRing(camera, mouse, center, dims);

    if (clickedRing != GIZMO_NONE) {
        currentGizmoState = clickedRing;
        return true;
    }

    float moveStart = dims.arrowStart;
    float moveEnd   = dims.arrowTipEnd;

    float scaleStart = dims.scaleHandleDistance - dims.cubeSize * 0.65f;
    float scaleEnd   = dims.scaleHandleDistance + dims.cubeSize * 0.65f;

    float axisThickness = dims.axisPickRadius;
    float scaleThickness = dims.cubeSize * 0.65f;

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
    Vector2 mousePosition = GetMousePosition();
    Ray ray = GetScreenToWorldRay(mousePosition, camera);

    bool clickedGizmo = false;
    float selectedMaxScale = GetSelectedMaxScale(
        cubes, cubeCount,
        spheres, sphereCount,
        cylinders, cylinderCount
    );

    GizmoDimensions gizmoDims = GetGizmoDimensions(selectedMaxScale);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        clickedGizmo = checkGizmoClick(ray, camera, mousePosition, gizmoCenter, gizmoDims);
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
    GizmoDimensions dims = GetGizmoDimensions(selectedMaxScale);

    
    //Move X
    DrawCylinderEx(
        Vector3Add(center, { dims.arrowStart, 0.0f, 0.0f }),
        Vector3Add(center, { dims.arrowLength, 0.0f, 0.0f }),
        dims.axisRadius,
        dims.axisRadius,
        12,
        RED
    );

    DrawCylinderEx(
        Vector3Add(center, { dims.arrowLength, 0.0f, 0.0f }),
        Vector3Add(center, { dims.arrowTipEnd, 0.0f, 0.0f }),
        dims.coneRadius,
        0.0f,
        12,
        RED
    );

    //Move Y
    DrawCylinderEx(
        Vector3Add(center, { 0.0f, dims.arrowStart, 0.0f }),
        Vector3Add(center, { 0.0f, dims.arrowLength, 0.0f }),
        dims.axisRadius,
        dims.axisRadius,
        12,
        GREEN
    );

    DrawCylinderEx(
        Vector3Add(center, { 0.0f, dims.arrowLength, 0.0f }),
        Vector3Add(center, { 0.0f, dims.arrowTipEnd, 0.0f }),
        dims.coneRadius,
        0.0f,
        12,
        GREEN
    );

    // Move Z
    DrawCylinderEx(
        Vector3Add(center, { 0.0f, 0.0f, dims.arrowStart }),
        Vector3Add(center, { 0.0f, 0.0f, dims.arrowLength }),
        dims.axisRadius,
        dims.axisRadius,
        12,
        BLUE
    );

    DrawCylinderEx(
        Vector3Add(center, { 0.0f, 0.0f, dims.arrowLength }),
        Vector3Add(center, { 0.0f, 0.0f, dims.arrowTipEnd }),
        dims.coneRadius,
        0.0f,
        12,
        BLUE
    );
    DrawCube(
        Vector3Add(center, { dims.scaleHandleDistance, 0.0f, 0.0f }),
        dims.cubeSize,
        dims.cubeSize,
        dims.cubeSize,
        RED
    );

    DrawCube(
        Vector3Add(center, { 0.0f, dims.scaleHandleDistance, 0.0f }),
        dims.cubeSize,
        dims.cubeSize,
        dims.cubeSize,
        GREEN
    );

    DrawCube(
        Vector3Add(center, { 0.0f, 0.0f, dims.scaleHandleDistance }),
        dims.cubeSize,
        dims.cubeSize,
        dims.cubeSize,
        BLUE
    );
    
    rlDisableDepthTest();

    rlSetLineWidth(3.0f);

    DrawCircle3D(
        center,
        dims.ringOuterX,
        { 0.0f, 1.0f, 0.0f },
        90.0f,
        RED
    );

    DrawCircle3D(
        center,
        dims.ringOuterY,
        { 1.0f, 0.0f, 0.0f },
        90.0f,
        GREEN
    );

    DrawCircle3D(
        center,
        dims.ringOuterZ,
        { 0.0f, 0.0f, 1.0f },
        0.0f,
        BLUE
    );

    rlSetLineWidth(2.0f);

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
