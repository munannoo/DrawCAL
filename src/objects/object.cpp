#include "object.h"

#include "data/save_load/saveNload.h"
#include "features/shadings/lighting.h"
#include "raymath.h"
#include "rlgl.h"
#include <float.h>

static Model cubeModel;
static Model sphereModel;
static Model cylinderModel;

static ObjectInstance Cu[100];
static ObjectInstance Sp[100];
static ObjectInstance cy[100];

enum ObjectType { NONE, CUBE, SPHERE, CYLINDER };

static void DrawObjectModel(Model& model, const ObjectInstance& obj, Color color);
static Matrix GetObjectTransform(const ObjectInstance& obj);
static void EnsureSceneVertices();
static void AssignDefaultVertices(ObjectInstance& obj, ObjectType type);
static void DrawSelectedVertexHandles(Camera3D camera);

static const float DEFAULT_LIGHT_INTENSITY = 350.0f;
static const float DEFAULT_LIGHT_RADIUS = 35.0f;
static const float DEFAULT_LIGHT_HEIGHT = 3.0f;
static int c = 0;
static int s = 0;
static int y = 0;

enum HandleRole { HANDLE_CORNER, HANDLE_CENTER, HANDLE_RADIUS, HANDLE_AXIS };

struct VertexHandle {
    Vector3 worldPos;
    HandleRole role;
};

void load() { loadScene(Cu, c, Sp, s, cy, y); }
void save() { saveScene(Cu, c, Sp, s, cy, y); }

static void SetMeshVertex(Mesh& mesh, int index, Vector3 position,
                          Vector3 normal, Vector2 uv)
{
    mesh.vertices[index * 3] = position.x;
    mesh.vertices[index * 3 + 1] = position.y;
    mesh.vertices[index * 3 + 2] = position.z;
    mesh.normals[index * 3] = normal.x;
    mesh.normals[index * 3 + 1] = normal.y;
    mesh.normals[index * 3 + 2] = normal.z;
    mesh.texcoords[index * 2] = uv.x;
    mesh.texcoords[index * 2 + 1] = uv.y;
}

static void AddTriangle(Mesh& mesh, int& index,
                        unsigned short a, unsigned short b, unsigned short c)
{
    mesh.indices[index++] = a;
    mesh.indices[index++] = b;
    mesh.indices[index++] = c;
}

static Mesh GenTexturedCylinder(float radius, float height, int slices)
{
    Mesh mesh = { 0 };

    if (slices < 3) slices = 3;

    float halfHeight = height * 0.5f;
    int totalVertexCount = (slices + 1) * 4 + 2;
    int totalTriangleCount = slices * 4;

    mesh.vertexCount = totalVertexCount;
    mesh.triangleCount = totalTriangleCount;

    mesh.vertices = (float*)MemAlloc(totalVertexCount * 3 * sizeof(float));
    mesh.normals = (float*)MemAlloc(totalVertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(totalVertexCount * 2 * sizeof(float));
    mesh.indices = (unsigned short*)MemAlloc(totalTriangleCount * 3 * sizeof(unsigned short));

    int v = 0;

    for (int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float angle = t * 2.0f * PI;

        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;

        Vector3 normal = Vector3Normalize({ x, 0.0f, z });

        SetMeshVertex(mesh, v++, { x, -halfHeight, z }, normal, { t, 1.0f });
        SetMeshVertex(mesh, v++, { x, halfHeight, z }, normal, { t, 0.0f });
    }

    int topCenterIndex = v;
    SetMeshVertex(mesh, v++, { 0.0f, halfHeight, 0.0f },
                  { 0.0f, 1.0f, 0.0f }, { 0.5f, 0.5f });
    int topRingStart = v;

    for (int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float angle = t * 2.0f * PI;

        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;

        float u = 0.5f + cosf(angle) * 0.5f;
        float vv = 0.5f + sinf(angle) * 0.5f;

        SetMeshVertex(mesh, v++, { x, halfHeight, z },
                      { 0.0f, 1.0f, 0.0f }, { u, vv });
    }

    int bottomCenterIndex = v;
    SetMeshVertex(mesh, v++, { 0.0f, -halfHeight, 0.0f },
                  { 0.0f, -1.0f, 0.0f }, { 0.5f, 0.5f });
    int bottomRingStart = v;

    for (int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float angle = t * 2.0f * PI;

        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;

        float u = 0.5f + cosf(angle) * 0.5f;
        float vv = 0.5f - sinf(angle) * 0.5f;

        SetMeshVertex(mesh, v++, { x, -halfHeight, z },
                      { 0.0f, -1.0f, 0.0f }, { u, vv });
    }

    int idx = 0;

    for (int i = 0; i < slices; i++)
    {
        unsigned short bottom0 = (unsigned short)(i * 2);
        unsigned short top0 = (unsigned short)(i * 2 + 1);
        unsigned short bottom1 = (unsigned short)((i + 1) * 2);
        unsigned short top1 = (unsigned short)((i + 1) * 2 + 1);

        AddTriangle(mesh, idx, bottom0, top0, top1);
        AddTriangle(mesh, idx, bottom0, top1, bottom1);
    }

    for (int i = 0; i < slices; i++)
    {
        unsigned short ring0 = (unsigned short)(topRingStart + i);
        unsigned short ring1 = (unsigned short)(topRingStart + i + 1);

        AddTriangle(mesh, idx, (unsigned short)topCenterIndex, ring1, ring0);
    }

    for (int i = 0; i < slices; i++)
    {
        unsigned short ring0 = (unsigned short)(bottomRingStart + i);
        unsigned short ring1 = (unsigned short)(bottomRingStart + i + 1);

        AddTriangle(mesh, idx, (unsigned short)bottomCenterIndex, ring0, ring1);
    }

    return mesh;
}

void initModels()
{
    InitLighting();

    Mesh cubeMesh = GenMeshCube(2.0f, 2.0f, 2.0f);
    GenMeshTangents(&cubeMesh);
    cubeModel = LoadModelFromMesh(cubeMesh);

    Mesh sphereMesh = GenMeshSphere(1.0f, 32, 32);
    GenMeshTangents(&sphereMesh);
    sphereModel = LoadModelFromMesh(sphereMesh);

    Mesh cylinderMesh = GenTexturedCylinder(1.0f, 2.0f, 64);
    GenMeshTangents(&cylinderMesh);
    UploadMesh(&cylinderMesh, false);

    cylinderModel = LoadModelFromMesh(cylinderMesh);
    LoadPBRTextures();

    ApplyLightingShader(cubeModel);
    ApplyLightingShader(sphereModel);
    ApplyLightingShader(cylinderModel);

    EnsureSceneVertices();
}

static ObjectInstance* GetObjectArray(int type)
{
    if (type == CUBE) return Cu;
    if (type == SPHERE) return Sp;
    if (type == CYLINDER) return cy;
    return nullptr;
}

ObjectInstance* getFirstSelectedMutable(int* outType, int* outIndex)
{
    for (int type = CUBE; type <= CYLINDER; type++)
    {
        ObjectInstance* objects = GetObjectArray(type);
        int count = getObjectCount(type);
        for (int i = 0; i < count; i++)
        {
            if (!objects[i].isSelected) continue;
            if (outType) *outType = type;
            if (outIndex) *outIndex = i;
            return &objects[i];
        }
    }

    if (outType) *outType = 0;
    if (outIndex) *outIndex = -1;

    return nullptr;
}

void deselectAllObjects()
{
    for (int type = CUBE; type <= CYLINDER; type++)
    {
        ObjectInstance* objects = GetObjectArray(type);
        for (int i = 0; i < getObjectCount(type); i++)
            objects[i].isSelected = false;
    }
}

void clear()
{
    for (int type = CUBE; type <= CYLINDER; type++)
    {
        ObjectInstance* objects = GetObjectArray(type);
        for (int i = 0; i < getObjectCount(type); i++)
            objects[i].isSelected = true;
    }
    deleteobj();
}

int getTotalSelectedCount()
{
    int total = 0;
    for (int type = CUBE; type <= CYLINDER; type++)
    {
        ObjectInstance* objects = GetObjectArray(type);
        for (int i = 0; i < getObjectCount(type); i++)
            if (objects[i].isSelected) total++;
    }
    return total;
}

// DefaultVertices -> gives local position of the shapes
static std::vector<Vector3> GetCubeDefaultVertices() {
    return {
        { -1.0f, -1.0f, -1.0f },
        {  1.0f, -1.0f, -1.0f },
        {  1.0f, -1.0f,  1.0f },
        { -1.0f, -1.0f,  1.0f },
        { -1.0f,  1.0f, -1.0f },
        {  1.0f,  1.0f, -1.0f },
        {  1.0f,  1.0f,  1.0f },
        { -1.0f,  1.0f,  1.0f },
    };
}

static std::vector<Vector3> GetSphereDefaultVertices() {
    return {
        {  0.0f,  0.0f,  0.0f },
        {  0.0f,  1.0f,  0.0f },
        {  0.0f, -1.0f,  0.0f },
        {  1.0f,  0.0f,  0.0f },
        { -1.0f,  0.0f,  0.0f },
        {  0.0f,  0.0f,  1.0f },
        {  0.0f,  0.0f, -1.0f },
    };
}

static std::vector<Vector3> GetCylinderDefaultVertices() {
    std::vector<Vector3> vertices;
    const int slices = 16;

    vertices.push_back({ 0.0f,  1.0f, 0.0f });
    vertices.push_back({ 0.0f, -1.0f, 0.0f });

    for (int i = 0; i < slices; i++) {
        float angle = (2.0f * PI * i) / slices;
        float x = cosf(angle);
        float z = sinf(angle);

        vertices.push_back({ x,  1.0f, z });
        vertices.push_back({ x, -1.0f, z });
    }

    return vertices;
}

static void AssignDefaultVertices(ObjectInstance& obj, ObjectType type)
{
    switch (type)
    {
        case CUBE: obj.vertices = GetCubeDefaultVertices(); break;
        case SPHERE: obj.vertices = GetSphereDefaultVertices(); break;
        case CYLINDER: obj.vertices = GetCylinderDefaultVertices(); break;
        default: obj.vertices.clear(); break;
    }
}

static void EnsureVertices(ObjectInstance& obj, ObjectType type)
{
    if (obj.vertices.empty()) AssignDefaultVertices(obj, type);
}

static void EnsureSceneVertices() {
    for (int type = CUBE; type <= CYLINDER; type++)
    {
        ObjectInstance* objects = GetObjectArray(type);
        for (int i = 0; i < getObjectCount(type); i++)
            EnsureVertices(objects[i], (ObjectType)type);
    }
}

static std::vector<VertexHandle> GetHandles(const ObjectInstance& obj, ObjectType type) {
    std::vector<VertexHandle> handles;
    Matrix transform = GetObjectTransform(obj);

    for (size_t i = 0; i < obj.vertices.size(); i++) {
        HandleRole role = HANDLE_CORNER;

        if (type == SPHERE) {
            role = (i == 0) ? HANDLE_CENTER : HANDLE_RADIUS;
        }
        else if (type == CYLINDER) {
            role = (i < 2) ? HANDLE_AXIS : HANDLE_RADIUS;
        }

        handles.push_back({ Vector3Transform(obj.vertices[i], transform), role });
    }

    return handles;
}

// checks if the vertices fall within the camera frame
static bool IsInFrontOfCamera(Vector3 position, Camera3D camera) {
    Vector3 cameraForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 toPosition = Vector3Subtract(position, camera.position);

    return Vector3DotProduct(cameraForward, toPosition) > 0.0f;
}

static void DrawHandle(Vector3 worldPos, HandleRole role, bool hovered) {
    Color color = WHITE;
    float radius = 0.055f;

    switch (role) {
        case HANDLE_CENTER:
            color = hovered ? YELLOW : GRAY;
            radius = 0.07f;
            break;
        case HANDLE_CORNER:
            color = hovered ? YELLOW : WHITE;
            break;
        case HANDLE_RADIUS:
            color = hovered ? ORANGE : WHITE;
            break;
        case HANDLE_AXIS:
            color = hovered ? SKYBLUE : BLUE;
            radius = 0.065f;
            break;
    }

    DrawSphere(worldPos, radius, color);
    DrawSphereWires(worldPos, radius * 1.35f, 8, 8, DARKGRAY);
}

static void DrawVertexHandlesForObject(const ObjectInstance& obj, ObjectType type, Camera3D camera) {
    Vector2 mouse = GetMousePosition();
    std::vector<VertexHandle> handles = GetHandles(obj, type);

    rlDisableDepthTest(); // draw over the objects.

    for (int i = 0; i < (int)handles.size(); i++) {
        VertexHandle& handle = handles[i];
        if (!IsInFrontOfCamera(handle.worldPos, camera)) continue;

        Vector2 screenPos = GetWorldToScreen(handle.worldPos, camera);
        bool hovered = CheckCollisionPointCircle(mouse, screenPos, 8.0f);

        DrawHandle(handle.worldPos, handle.role, hovered);
    }

    rlEnableDepthTest();
}

static void DrawSelectedVertexHandles(Camera3D camera) {
    EnsureSceneVertices();

    for (int type = CUBE; type <= CYLINDER; type++)
    {
        ObjectInstance* objects = GetObjectArray(type);
        for (int i = 0; i < getObjectCount(type); i++)
            if (objects[i].isSelected)
                DrawVertexHandlesForObject(objects[i], (ObjectType)type, camera);
    }
}

static void AddObject(ObjectInstance objects[], int& count, ObjectType type,
                      Vector3 position, Color color)
{
    if (count >= 100) return;

    ObjectInstance& object = objects[count];
    object.position = position;
    object.rotation = { 0.0f, 0.0f, 0.0f };
    object.scale = { 1.0f, 1.0f, 1.0f };
    object.color = color;
    object.isSelected = false;
    object.isLight = false;
    object.lightIndex = -1;
    object.lightIntensity = 0.0f;
    object.lightRadius = 0.0f;
    object.material = MATERIAL_WOOD;
    AssignDefaultVertices(object, type);
    count++;
}

void cube(const Vector3 pos, Color color) { AddObject(Cu, c, CUBE, pos, color); }
void sphere(const Vector3 pos, Color color) { AddObject(Sp, s, SPHERE, pos, color); }
void cylinder(const Vector3 pos, Color color) { AddObject(cy, y, CYLINDER, pos, color); }

void SyncObjectLightsToScene()
{
    for (int i = 0; i < s; i++)
    {
        if (!Sp[i].isLight || Sp[i].lightIndex < 0) continue;

        SetSceneLightPosition(Sp[i].lightIndex, Sp[i].position);
        SetSceneLightProperties(Sp[i].lightIndex, Sp[i].color,
                                Sp[i].lightIntensity, Sp[i].lightRadius);
    }
}

static void RenderObjects(Model& model, ObjectInstance objects[], int count)
{
    for (int i = 0; i < count; i++)
    {
        Color color = objects[i].color;
        color.a = objects[i].isSelected ? 128 : 255;
        DrawObjectModel(model, objects[i], color);
    }
}

void renderCube() { RenderObjects(cubeModel, Cu, c); }
void renderCylinder() { RenderObjects(cylinderModel, cy, y); }

void renderSphere()
{
    for (int i = 0; i < s; i++)
    {
        if (Sp[i].isLight && Sp[i].lightIndex >= 0)
        {
            SetSceneLightPosition(Sp[i].lightIndex, Sp[i].position);
            SetSceneLightProperties(Sp[i].lightIndex, Sp[i].color,
                                    Sp[i].lightIntensity, Sp[i].lightRadius);
            DrawSphere(Sp[i].position, Sp[i].scale.x, Sp[i].color);
            if (Sp[i].isSelected)
                DrawSphereWires(Sp[i].position, Sp[i].scale.x * 1.15f, 16, 16, WHITE);
            continue;
        }

        Color color = Sp[i].color;
        color.a = Sp[i].isSelected ? 128 : 255;
        DrawObjectModel(sphereModel, Sp[i], color);
    }
}

static void FindClosestHit(Ray ray, Model& model, ObjectInstance objects[],
                           int count, ObjectType type, float& closestDistance,
                           int& closestIndex, ObjectType& closestType)
{
    for (int i = 0; i < count; i++)
    {
        RayCollision hit = GetRayCollisionMesh(
            ray, model.meshes[0], GetObjectTransform(objects[i]));
        if (hit.hit && hit.distance < closestDistance)
        {
            closestDistance = hit.distance;
            closestIndex = i;
            closestType = type;
        }
    }
}

void leftclick(Ray ray)
{
    int closestIndex = -1;
    ObjectType closestType = NONE;
    float closestDistance = FLT_MAX;
    FindClosestHit(ray, cubeModel, Cu, c, CUBE,
                   closestDistance, closestIndex, closestType);
    FindClosestHit(ray, sphereModel, Sp, s, SPHERE,
                   closestDistance, closestIndex, closestType);
    FindClosestHit(ray, cylinderModel, cy, y, CYLINDER,
                   closestDistance, closestIndex, closestType);

    bool additive = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    if (!additive) deselectAllObjects();
    if (closestType != NONE)
    {
        ObjectInstance* object = getObjectMutable((int)closestType, closestIndex);
        object->isSelected = additive ? !object->isSelected : true;
    }
}

static Matrix GetObjectTransform(const ObjectInstance& obj)
{
    Matrix matScale = MatrixScale(obj.scale.x, obj.scale.y, obj.scale.z);
    Matrix matRotation = MatrixRotateXYZ({
        DEG2RAD * obj.rotation.x,
        DEG2RAD * obj.rotation.y,
        DEG2RAD * obj.rotation.z
    });
    Matrix translation = MatrixTranslate(
        obj.position.x, obj.position.y, obj.position.z);
    return MatrixMultiply(MatrixMultiply(matScale, matRotation), translation);
}

static void DrawObjectModel(Model& model, const ObjectInstance& obj, Color color)
{
    ApplyPBRMaterial(model, obj.material);

    Model tempModel = model;
    tempModel.transform = GetObjectTransform(obj);

    DrawModel(tempModel, { 0.0f, 0.0f, 0.0f }, 1.0f, color);
}

bool updateObjectTransformGizmo(Camera3D camera) {
    return UpdateTransformGizmo(camera, Cu, c, Sp, s, cy, y);
}

void drawObjectTransformGizmo(Camera3D camera) {
    DrawTransformGizmo(Cu, c, Sp, s, cy, y);
    DrawSelectedVertexHandles(camera);
}

void lightSphere(const Vector3 pos, Color color)
{
    if (s >= 100) return;

    Vector3 lightPos = pos;
    lightPos.y += DEFAULT_LIGHT_HEIGHT;
    int lightIndex = CreatePointLight(lightPos);
    if (lightIndex == -1) return;

    int index = s;
    AddObject(Sp, s, SPHERE, lightPos, color);
    Sp[index].scale = { 0.25f, 0.25f, 0.25f };
    Sp[index].isLight = true;
    Sp[index].lightIndex = lightIndex;
    Sp[index].lightIntensity = DEFAULT_LIGHT_INTENSITY;
    Sp[index].lightRadius = DEFAULT_LIGHT_RADIUS;
    SetSceneLightProperties(lightIndex, color, DEFAULT_LIGHT_INTENSITY,
                            DEFAULT_LIGHT_RADIUS);
}

static void RemoveObject(ObjectInstance objects[], int& count, int index)
{
    for (int i = index; i < count - 1; i++)
        objects[i] = objects[i + 1];
    count--;
}

static void DeleteSelectedObjects(ObjectInstance objects[], int& count)
{
    for (int i = 0; i < count; i++)
    {
        if (!objects[i].isSelected) continue;
        RemoveObject(objects, count, i);
        i--;
    }
}

static void DeleteSelectedSpheres()
{
    for (int i = 0; i < s; i++)
    {
        if (!Sp[i].isSelected) continue;

        if (Sp[i].isLight && Sp[i].lightIndex >= 0)
        {
            int lightIndex = Sp[i].lightIndex;
            DeleteSceneLight(lightIndex);
            for (int k = 0; k < s; k++)
                if (Sp[k].isLight && Sp[k].lightIndex > lightIndex)
                    Sp[k].lightIndex--;
        }

        RemoveObject(Sp, s, i);
        i--;
    }
}

void deleteobj()
{
    DeleteSelectedObjects(Cu, c);
    DeleteSelectedSpheres();
    DeleteSelectedObjects(cy, y);
}

void unloadModels()
{
    UnloadModel(cubeModel);
    UnloadModel(sphereModel);
    UnloadModel(cylinderModel);
}

int getObjectCount(int objectType)
{
    switch (objectType)
    {
        case CUBE: return c;
        case SPHERE: return s;
        case CYLINDER: return y;
        default: return 0;
    }
}

ObjectInstance* getObjectMutable(int objectType, int objectIndex)
{
    if (objectIndex < 0 || objectIndex >= getObjectCount(objectType)) return nullptr;
    return &GetObjectArray(objectType)[objectIndex];
}

bool selectObject(int objectType, int objectIndex, bool additive)
{
    ObjectInstance* object = getObjectMutable(objectType, objectIndex);
    if (object == nullptr) return false;

    if (!additive) deselectAllObjects();
    object->isSelected = additive ? !object->isSelected : true;
    return true;
}
