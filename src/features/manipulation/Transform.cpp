#include "Transform.h"

#include "math.h"
#include "raymath.h"
#include "rlgl.h"

static GizmoState currentGizmoState = GIZMO_NONE;
static Vector3 gizmoCenter = { 0.0f, 0.0f, 0.0f };
static Model ringModel;
static bool ringModelLoaded = false;

struct ObjectGroup
{
    ObjectInstance* objects;
    int count;
};

struct GizmoDimensions
{
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
};

float ClampFloat(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

void MakeGroups(ObjectGroup groups[3],
                ObjectInstance* cubes, int cubeCount,
                ObjectInstance* spheres, int sphereCount,
                ObjectInstance* cylinders, int cylinderCount)
{
    groups[0] = { cubes, cubeCount };
    groups[1] = { spheres, sphereCount };
    groups[2] = { cylinders, cylinderCount };
}

GizmoDimensions GetGizmoDimensions(float maxScale)
{
    GizmoDimensions dims = { 0 };
    dims.size = ClampFloat(1.0f + (maxScale - 1.0f) * 0.25f, 0.9f, 6.0f);
    dims.objectRadius = fmaxf(maxScale * 1.25f, 1.1f);
    dims.ringOuterX = dims.objectRadius + 0.55f * dims.size;
    dims.ringOuterY = dims.objectRadius + 0.78f * dims.size;
    dims.ringOuterZ = dims.objectRadius + 1.01f * dims.size;
    dims.arrowStart = dims.ringOuterZ + 0.35f * dims.size;
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
    ringModel = LoadModelFromMesh(GenMeshTorus(0.8f, 4.0f, 16, 32));
    ringModelLoaded = true;
}

void UnloadTransformGizmo()
{
    if (!ringModelLoaded) return;
    UnloadModel(ringModel);
    ringModelLoaded = false;
}

bool HasSelectedObject(ObjectGroup groups[3])
{
    for (int group = 0; group < 3; group++)
        for (int i = 0; i < groups[group].count; i++)
            if (groups[group].objects[i].isSelected) return true;
    return false;
}

Vector3 GetGizmoCenter(ObjectGroup groups[3])
{
    Vector3 center = { 0.0f, 0.0f, 0.0f };
    int selectedCount = 0;

    for (int group = 0; group < 3; group++)
    {
        for (int i = 0; i < groups[group].count; i++)
        {
            ObjectInstance& object = groups[group].objects[i];
            if (!object.isSelected) continue;
            center = Vector3Add(center, object.position);
            selectedCount++;
        }
    }

    if (selectedCount > 0) center = Vector3Scale(center, 1.0f / selectedCount);
    return center;
}

float GetSelectedMaxScale(ObjectGroup groups[3])
{
    float maxScale = 1.0f;

    for (int group = 0; group < 3; group++)
    {
        for (int i = 0; i < groups[group].count; i++)
        {
            ObjectInstance& object = groups[group].objects[i];
            if (!object.isSelected) continue;
            maxScale = fmaxf(maxScale, object.scale.x);
            maxScale = fmaxf(maxScale, object.scale.y);
            maxScale = fmaxf(maxScale, object.scale.z);
        }
    }
    return maxScale;
}

void TransformSelected(ObjectGroup groups[3], Vector3 amount, int action)
{
    for (int group = 0; group < 3; group++)
    {
        for (int i = 0; i < groups[group].count; i++)
        {
            ObjectInstance& object = groups[group].objects[i];
            if (!object.isSelected) continue;

            if (action == 0) object.position = Vector3Add(object.position, amount);
            else if (action == 1)
            {
                object.scale = Vector3Add(object.scale, amount);
                if (object.scale.x < 0.1f) object.scale.x = 0.1f;
                if (object.scale.y < 0.1f) object.scale.y = 0.1f;
                if (object.scale.z < 0.1f) object.scale.z = 0.1f;
            }
            else object.rotation = Vector3Add(object.rotation, amount);
        }
    }
}

float DistancePointToSegment(Vector2 point, Vector2 start, Vector2 end)
{
    Vector2 segment = Vector2Subtract(end, start);
    float lengthSquared = Vector2DotProduct(segment, segment);
    if (lengthSquared <= 0.0001f) return Vector2Distance(point, start);

    float t = Vector2DotProduct(Vector2Subtract(point, start), segment) / lengthSquared;
    t = ClampFloat(t, 0.0f, 1.0f);
    Vector2 closest = Vector2Add(start, Vector2Scale(segment, t));
    return Vector2Distance(point, closest);
}

float GetRingScreenDistance(Camera3D camera, Vector2 mouse, Vector3 center,
                            Vector3 axisA, Vector3 axisB, float radius)
{
    float closest = 1000000.0f;
    int segments = 96;

    for (int i = 0; i < segments; i++)
    {
        float angleA = 2.0f * PI * i / segments;
        float angleB = 2.0f * PI * (i + 1) / segments;
        Vector3 pointA = Vector3Add(
            center, Vector3Add(Vector3Scale(axisA, cosf(angleA) * radius),
                               Vector3Scale(axisB, sinf(angleA) * radius)));
        Vector3 pointB = Vector3Add(
            center, Vector3Add(Vector3Scale(axisA, cosf(angleB) * radius),
                               Vector3Scale(axisB, sinf(angleB) * radius)));
        float distance = DistancePointToSegment(
            mouse, GetWorldToScreen(pointA, camera), GetWorldToScreen(pointB, camera));
        if (distance < closest) closest = distance;
    }
    return closest;
}

GizmoState GetClickedRing(Camera3D camera, Vector2 mouse, Vector3 center,
                          GizmoDimensions dims)
{
    Vector3 axisA[3] = {
        { 0.0f, 1.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f }
    };
    Vector3 axisB[3] = {
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f }
    };
    float radius[3] = { dims.ringOuterX, dims.ringOuterY, dims.ringOuterZ };
    float closest = dims.ringPickPixels;
    GizmoState result = GIZMO_NONE;

    for (int axis = 0; axis < 3; axis++)
    {
        float distance = GetRingScreenDistance(
            camera, mouse, center, axisA[axis], axisB[axis], radius[axis]);
        if (distance >= closest) continue;
        closest = distance;
        result = (GizmoState)(GIZMO_ROTATE_X + axis);
    }
    return result;
}

BoundingBox MakeAxisBox(Vector3 center, int axis, float start, float end,
                        float thickness)
{
    BoundingBox box = {
        { center.x - thickness, center.y - thickness, center.z - thickness },
        { center.x + thickness, center.y + thickness, center.z + thickness }
    };

    if (axis == 0) { box.min.x = center.x + start; box.max.x = center.x + end; }
    if (axis == 1) { box.min.y = center.y + start; box.max.y = center.y + end; }
    if (axis == 2) { box.min.z = center.z + start; box.max.z = center.z + end; }
    return box;
}

bool CheckGizmoClick(Ray ray, Camera3D camera, Vector2 mouse, Vector3 center,
                     GizmoDimensions dims)
{
    currentGizmoState = GetClickedRing(camera, mouse, center, dims);
    if (currentGizmoState != GIZMO_NONE) return true;

    for (int axis = 0; axis < 3; axis++)
    {
        BoundingBox moveBox = MakeAxisBox(
            center, axis, dims.arrowStart, dims.arrowTipEnd, dims.axisPickRadius);
        if (GetRayCollisionBox(ray, moveBox).hit)
        {
            currentGizmoState = (GizmoState)(GIZMO_MOVE_X + axis);
            return true;
        }
    }

    float start = dims.scaleHandleDistance - dims.cubeSize * 0.65f;
    float end = dims.scaleHandleDistance + dims.cubeSize * 0.65f;
    float thickness = dims.cubeSize * 0.65f;
    for (int axis = 0; axis < 3; axis++)
    {
        if (GetRayCollisionBox(ray, MakeAxisBox(center, axis, start, end, thickness)).hit)
        {
            currentGizmoState = (GizmoState)(GIZMO_SCALE_X + axis);
            return true;
        }
    }
    return false;
}

void ApplyGizmoDrag(ObjectGroup groups[3], Vector2 mouseDelta)
{
    Vector3 amount = { 0.0f, 0.0f, 0.0f };
    int action;
    int axis;
    float value;

    if (currentGizmoState >= GIZMO_MOVE_X && currentGizmoState <= GIZMO_MOVE_Z)
    {
        action = 0;
        axis = currentGizmoState - GIZMO_MOVE_X;
        value = axis == 1 ? -mouseDelta.y * 0.03f : mouseDelta.x * 0.03f;
        if (axis == 2) value = -value;
    }
    else if (currentGizmoState >= GIZMO_SCALE_X && currentGizmoState <= GIZMO_SCALE_Z)
    {
        action = 1;
        axis = currentGizmoState - GIZMO_SCALE_X;
        value = axis == 1 ? -mouseDelta.y * 0.01f : mouseDelta.x * 0.01f;
        if (axis == 2) value = -value;
    }
    else
    {
        action = 2;
        axis = currentGizmoState - GIZMO_ROTATE_X;
        value = mouseDelta.x * 0.5f;
    }

    if (axis == 0) amount.x = value;
    if (axis == 1) amount.y = value;
    if (axis == 2) amount.z = value;
    TransformSelected(groups, amount, action);
}

bool UpdateTransformGizmo(Camera3D camera,
                          ObjectInstance* cubes, int cubeCount,
                          ObjectInstance* spheres, int sphereCount,
                          ObjectInstance* cylinders, int cylinderCount)
{
    ObjectGroup groups[3];
    MakeGroups(groups, cubes, cubeCount, spheres, sphereCount,
               cylinders, cylinderCount);

    if (!HasSelectedObject(groups))
    {
        currentGizmoState = GIZMO_NONE;
        return false;
    }

    gizmoCenter = GetGizmoCenter(groups);
    Vector2 mouse = GetMousePosition();
    GizmoDimensions dims = GetGizmoDimensions(GetSelectedMaxScale(groups));
    bool clicked = false;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        clicked = CheckGizmoClick(GetScreenToWorldRay(mouse, camera), camera,
                                  mouse, gizmoCenter, dims);

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && currentGizmoState != GIZMO_NONE)
    {
        ApplyGizmoDrag(groups, GetMouseDelta());
        return true;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        currentGizmoState = GIZMO_NONE;

    return clicked;
}

void DrawAxisGizmo(Vector3 center, Vector3 axis, Color color,
                   GizmoDimensions dims)
{
    DrawCylinderEx(Vector3Add(center, Vector3Scale(axis, dims.arrowStart)),
                   Vector3Add(center, Vector3Scale(axis, dims.arrowLength)),
                   dims.axisRadius, dims.axisRadius, 12, color);
    DrawCylinderEx(Vector3Add(center, Vector3Scale(axis, dims.arrowLength)),
                   Vector3Add(center, Vector3Scale(axis, dims.arrowTipEnd)),
                   dims.coneRadius, 0.0f, 12, color);
    Vector3 handle = Vector3Add(
        center, Vector3Scale(axis, dims.scaleHandleDistance));
    DrawCube(handle, dims.cubeSize, dims.cubeSize, dims.cubeSize, color);
}

void DrawTransformGizmo(ObjectInstance* cubes, int cubeCount,
                        ObjectInstance* spheres, int sphereCount,
                        ObjectInstance* cylinders, int cylinderCount)
{
    ObjectGroup groups[3];
    MakeGroups(groups, cubes, cubeCount, spheres, sphereCount,
               cylinders, cylinderCount);
    if (!HasSelectedObject(groups)) return;

    Vector3 center = GetGizmoCenter(groups);
    GizmoDimensions dims = GetGizmoDimensions(GetSelectedMaxScale(groups));
    Vector3 axes[3] = {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f }
    };
    Color colors[3] = { RED, GREEN, BLUE };

    for (int axis = 0; axis < 3; axis++)
        DrawAxisGizmo(center, axes[axis], colors[axis], dims);

    rlDisableDepthTest();
    rlSetLineWidth(3.0f);
    DrawCircle3D(center, dims.ringOuterX, { 0.0f, 1.0f, 0.0f }, 90.0f, RED);
    DrawCircle3D(center, dims.ringOuterY, { 1.0f, 0.0f, 0.0f }, 90.0f, GREEN);
    DrawCircle3D(center, dims.ringOuterZ, { 0.0f, 0.0f, 1.0f }, 0.0f, BLUE);
    rlSetLineWidth(2.0f);
    rlEnableDepthTest();
}
