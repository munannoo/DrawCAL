#include <raylib.h>
#include "Transform.h"
#include "raymath.h"
#include "rlgl.h"
#include "math.h"
#include "objects/object.h"   // shape, selectedObjects, activeObject

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
    dims.ringOuterX = dims.objectRadius + 0.55f * dims.size;
    dims.ringOuterY = dims.objectRadius + 0.78f * dims.size;
    dims.ringOuterZ = dims.objectRadius + 1.01f * dims.size;
    float outerRing = dims.ringOuterZ;
    dims.arrowStart = outerRing + 0.35f * dims.size;
    dims.arrowLength = dims.arrowStart + 1.45f * dims.size;
    dims.arrowTipEnd = dims.arrowLength + 0.45f * dims.size;
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

// --- Selection-driven helpers, now working on the real object system ---

static Vector3 GetGizmoCenter()
{
    Vector3 center = { 0.0f, 0.0f, 0.0f };
    int count = 0;

    for (shape* obj : selectedObjects)
    {
        center = Vector3Add(center, obj->getTransform().translation);
        count++;
    }

    if (count > 0) center = Vector3Scale(center, 1.0f / count);
    return center;
}

static float GetSelectedMaxScale()
{
    float maxScale = 1.0f;
    for (shape* obj : selectedObjects)
    {
        Vector3 s = obj->getTransform().scale;
        maxScale = fmaxf(maxScale, fmaxf(s.x, fmaxf(s.y, s.z)));
    }
    return maxScale;
}

static void MoveSelectedObjects(Vector3 delta)
{
    for (shape* obj : selectedObjects)
    {
        Transform t = obj->getTransform();
        t.translation = Vector3Add(t.translation, delta);
        obj->setTransform(t);
    }
}

static void ScaleSelectedObjects(Vector3 scaleDelta)
{
    for (shape* obj : selectedObjects)
    {
        Transform t = obj->getTransform();
        t.scale = Vector3Add(t.scale, scaleDelta);
        t.scale.x = fmaxf(t.scale.x, 0.1f);
        t.scale.y = fmaxf(t.scale.y, 0.1f);
        t.scale.z = fmaxf(t.scale.z, 0.1f);
        obj->setTransform(t);
    }
}

// Rotation deltas here are in degrees (kept consistent with the old API); shape
// stores rotation as a quaternion, so we round-trip through Euler degrees.
static void RotateSelectedObjectsAxis(Vector3 rotationDeltaDeg)
{
    for (shape* obj : selectedObjects)
    {
        Transform t = obj->getTransform();
        Vector3 eulerDeg = Vector3Scale(QuaternionToEuler(t.rotation), RAD2DEG);
        eulerDeg = Vector3Add(eulerDeg, rotationDeltaDeg);
        Vector3 eulerRad = Vector3Scale(eulerDeg, DEG2RAD);
        t.rotation = QuaternionFromEuler(eulerRad.x, eulerRad.y, eulerRad.z);
        obj->setTransform(t);
    }
}

// --- Screen-space picking, now viewport-aware instead of assuming full window ---

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

// `mouseLocal` and the returned distances are all in the viewport's own local
// pixel space (0,0 at the viewport's top-left) — caller is responsible for
// converting screen-space mouse position into that space before calling in.
static float GetRingScreenDistance(
    Camera3D camera,
    Rectangle viewport,
    Vector2 mouseLocal,
    Vector3 center,
    Vector3 axisA,
    Vector3 axisB,
    float radius
) {
    float closestDistance = 1000000.0f;
    const int segments = 96;
    int vw = (int)viewport.width;
    int vh = (int)viewport.height;

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

        // viewport-aware — was GetWorldToScreen(world, camera), which assumes
        // a full-window viewport and gives wrong screen positions in split-screen.
        Vector2 screenA = GetWorldToScreenEx(worldA, camera, vw, vh);
        Vector2 screenB = GetWorldToScreenEx(worldB, camera, vw, vh);
        float distance = DistancePointToSegment(mouseLocal, screenA, screenB);

        if (distance < closestDistance) {
            closestDistance = distance;
        }
    }

    return closestDistance;
}

static GizmoState GetClickedRing(Camera3D camera, Rectangle viewport, Vector2 mouseLocal, Vector3 center, GizmoDimensions dims)
{
    float closestRingDistance = dims.ringPickPixels;
    GizmoState closestRing = GIZMO_NONE;

    float ringDistance = GetRingScreenDistance(camera, viewport, mouseLocal, center,
        { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, dims.ringOuterX);
    if (ringDistance < closestRingDistance) { closestRingDistance = ringDistance; closestRing = GIZMO_ROTATE_X; }

    ringDistance = GetRingScreenDistance(camera, viewport, mouseLocal, center,
        { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, dims.ringOuterY);
    if (ringDistance < closestRingDistance) { closestRingDistance = ringDistance; closestRing = GIZMO_ROTATE_Y; }

    ringDistance = GetRingScreenDistance(camera, viewport, mouseLocal, center,
        { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, dims.ringOuterZ);
    if (ringDistance < closestRingDistance) { closestRingDistance = ringDistance; closestRing = GIZMO_ROTATE_Z; }

    return closestRing;
}

static bool checkGizmoClick(Ray ray, Camera3D camera, Rectangle viewport, Vector2 mouseLocal, Vector3 center, GizmoDimensions dims) {
    GizmoState clickedRing = GetClickedRing(camera, viewport, mouseLocal, center, dims);

    if (clickedRing != GIZMO_NONE) {
        currentGizmoState = clickedRing;
        return true;
    }

    float moveStart = dims.arrowStart;
    float moveEnd = dims.arrowTipEnd;
    float scaleStart = dims.scaleHandleDistance - dims.cubeSize * 0.65f;
    float scaleEnd = dims.scaleHandleDistance + dims.cubeSize * 0.65f;
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

    if (GetRayCollisionBox(ray, moveXBox).hit) { currentGizmoState = GIZMO_MOVE_X; return true; }
    if (GetRayCollisionBox(ray, moveYBox).hit) { currentGizmoState = GIZMO_MOVE_Y; return true; }
    if (GetRayCollisionBox(ray, moveZBox).hit) { currentGizmoState = GIZMO_MOVE_Z; return true; }

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

    if (GetRayCollisionBox(ray, scaleXBox).hit) { currentGizmoState = GIZMO_SCALE_X; return true; }
    if (GetRayCollisionBox(ray, scaleYBox).hit) { currentGizmoState = GIZMO_SCALE_Y; return true; }
    if (GetRayCollisionBox(ray, scaleZBox).hit) { currentGizmoState = GIZMO_SCALE_Z; return true; }

    return false;
}

// `viewport` is the SCREEN-SPACE rect of whichever viewport this camera belongs
// to (same one computeViewportBounds() produces) — needed so the mouse ray and
// ring-picking use this viewport's own dimensions/offset, not the full window.
bool UpdateTransformGizmo(Camera3D camera, Rectangle viewport)
{
    if (selectedObjects.empty()) {
        currentGizmoState = GIZMO_NONE;
        return false;
    }

    gizmoCenter = GetGizmoCenter();

    Vector2 mouseDelta = GetMouseDelta();
    Vector2 mouseScreen = GetMousePosition();
    Vector2 mouseLocal = { mouseScreen.x - viewport.x, mouseScreen.y - viewport.y };

    // viewport-aware — was GetScreenToWorldRay(mouseScreen, camera)
    Ray ray = GetScreenToWorldRayEx(mouseLocal, camera, (int)viewport.width, (int)viewport.height);

    bool clickedGizmo = false;
    float selectedMaxScale = GetSelectedMaxScale();
    GizmoDimensions gizmoDims = GetGizmoDimensions(selectedMaxScale);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        clickedGizmo = checkGizmoClick(ray, camera, viewport, mouseLocal, gizmoCenter, gizmoDims);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && currentGizmoState != GIZMO_NONE) {
        float moveSensitivity = 0.03f;
        float scaleSensitivity = 0.01f;
        float rotationSensitivity = 0.5f;

        switch (currentGizmoState) {
        case GIZMO_MOVE_X:   MoveSelectedObjects({ mouseDelta.x * moveSensitivity, 0.0f, 0.0f }); break;
        case GIZMO_MOVE_Y:   MoveSelectedObjects({ 0.0f, -mouseDelta.y * moveSensitivity, 0.0f }); break;
        case GIZMO_MOVE_Z:   MoveSelectedObjects({ 0.0f, 0.0f, -mouseDelta.x * moveSensitivity }); break;
        case GIZMO_SCALE_X:  ScaleSelectedObjects({ mouseDelta.x * scaleSensitivity, 0.0f, 0.0f }); break;
        case GIZMO_SCALE_Y:  ScaleSelectedObjects({ 0.0f, -mouseDelta.y * scaleSensitivity, 0.0f }); break;
        case GIZMO_SCALE_Z:  ScaleSelectedObjects({ 0.0f, 0.0f, -mouseDelta.x * scaleSensitivity }); break;
        case GIZMO_ROTATE_X: RotateSelectedObjectsAxis({ mouseDelta.x * rotationSensitivity, 0.0f, 0.0f }); break;
        case GIZMO_ROTATE_Y: RotateSelectedObjectsAxis({ 0.0f, mouseDelta.x * rotationSensitivity, 0.0f }); break;
        case GIZMO_ROTATE_Z: RotateSelectedObjectsAxis({ 0.0f, 0.0f, mouseDelta.x * rotationSensitivity }); break;
        default: break;
        }

        return true;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        currentGizmoState = GIZMO_NONE;
    }

    return clickedGizmo;
}

void DrawTransformGizmo()
{
    if (selectedObjects.empty()) return;

    Vector3 center = GetGizmoCenter();
    float selectedMaxScale = GetSelectedMaxScale();
    GizmoDimensions dims = GetGizmoDimensions(selectedMaxScale);

    // Move X
    DrawCylinderEx(Vector3Add(center, { dims.arrowStart, 0, 0 }), Vector3Add(center, { dims.arrowLength, 0, 0 }), dims.axisRadius, dims.axisRadius, 12, RED);
    DrawCylinderEx(Vector3Add(center, { dims.arrowLength, 0, 0 }), Vector3Add(center, { dims.arrowTipEnd, 0, 0 }), dims.coneRadius, 0.0f, 12, RED);
    // Move Y
    DrawCylinderEx(Vector3Add(center, { 0, dims.arrowStart, 0 }), Vector3Add(center, { 0, dims.arrowLength, 0 }), dims.axisRadius, dims.axisRadius, 12, GREEN);
    DrawCylinderEx(Vector3Add(center, { 0, dims.arrowLength, 0 }), Vector3Add(center, { 0, dims.arrowTipEnd, 0 }), dims.coneRadius, 0.0f, 12, GREEN);
    // Move Z
    DrawCylinderEx(Vector3Add(center, { 0, 0, dims.arrowStart }), Vector3Add(center, { 0, 0, dims.arrowLength }), dims.axisRadius, dims.axisRadius, 12, BLUE);
    DrawCylinderEx(Vector3Add(center, { 0, 0, dims.arrowLength }), Vector3Add(center, { 0, 0, dims.arrowTipEnd }), dims.coneRadius, 0.0f, 12, BLUE);

    DrawCube(Vector3Add(center, { dims.scaleHandleDistance, 0, 0 }), dims.cubeSize, dims.cubeSize, dims.cubeSize, RED);
    DrawCube(Vector3Add(center, { 0, dims.scaleHandleDistance, 0 }), dims.cubeSize, dims.cubeSize, dims.cubeSize, GREEN);
    DrawCube(Vector3Add(center, { 0, 0, dims.scaleHandleDistance }), dims.cubeSize, dims.cubeSize, dims.cubeSize, BLUE);

    rlDisableDepthTest();
    rlSetLineWidth(3.0f);
    DrawCircle3D(center, dims.ringOuterX, { 0.0f, 1.0f, 0.0f }, 90.0f, RED);
    DrawCircle3D(center, dims.ringOuterY, { 1.0f, 0.0f, 0.0f }, 90.0f, GREEN);
    DrawCircle3D(center, dims.ringOuterZ, { 0.0f, 0.0f, 1.0f }, 0.0f, BLUE);
    rlSetLineWidth(2.0f);
    rlEnableDepthTest();
}