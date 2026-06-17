#include "object.h"
#include "features/manipulation/Transform.h"
#include "raylib.h"
#include <float.h>
#include "raymath.h"
#include "rlgl.h"
#include "features/shadings/textures.h"
#include "features/shadings/lighting.h"
#include "data/save_load/saveNload.h"
#include <iostream>
#include "features/shadings/textures.h"
static Model cubeModel;
static Model sphereModel;
static Model cylinderModel;

static BoundingBox Cubebounds;
static BoundingBox Spherebounds;
static BoundingBox Cylinderbounds;

static ObjectInstance Cu[100];
static ObjectInstance Sp[100];
static ObjectInstance cy[100];

static void DrawObjectModel(Model &model, ObjectInstance obj, Color color);

enum ObjectType { NONE, CUBE, SPHERE, CYLINDER };
static ObjectType selectedType = NONE;
static int c = 0;
static int s = 0;
static int y = 0;
static int totalSelectedCount = 0;

void load(){
    loadScene(Cu, c, Sp, s, cy, y);
}
void save(){
    saveScene(Cu, c, Sp, s, cy, y);
}
static Mesh GenTexturedCylinder(float radius, float height, int slices)
{
    Mesh mesh = { 0 };

    if (slices < 3) slices = 3;

    float halfHeight = height * 0.5f;

    int sideVertexCount = (slices + 1) * 2;
    int topVertexCount = slices + 2;     
    int bottomVertexCount = slices + 2;  
    int totalVertexCount = sideVertexCount + topVertexCount + bottomVertexCount;

    int sideTriangleCount = slices * 2;
    int topTriangleCount = slices;
    int bottomTriangleCount = slices;

    int totalTriangleCount =
        sideTriangleCount + topTriangleCount + bottomTriangleCount;

    mesh.vertexCount = totalVertexCount;
    mesh.triangleCount = totalTriangleCount;

    mesh.vertices = (float*)MemAlloc(totalVertexCount * 3 * sizeof(float));
    mesh.normals = (float*)MemAlloc(totalVertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(totalVertexCount * 2 * sizeof(float));
    mesh.indices = (unsigned short*)MemAlloc(totalTriangleCount * 3 * sizeof(unsigned short));

    int v = 0;

    auto SetVertex = [&](int index, Vector3 position, Vector3 normal, Vector2 uv)
    {
        mesh.vertices[index * 3 + 0] = position.x;
        mesh.vertices[index * 3 + 1] = position.y;
        mesh.vertices[index * 3 + 2] = position.z;

        mesh.normals[index * 3 + 0] = normal.x;
        mesh.normals[index * 3 + 1] = normal.y;
        mesh.normals[index * 3 + 2] = normal.z;

        mesh.texcoords[index * 2 + 0] = uv.x;
        mesh.texcoords[index * 2 + 1] = uv.y;
    };

    for (int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float angle = t * 2.0f * PI;

        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;

        Vector3 normal = Vector3Normalize({ x, 0.0f, z });

        SetVertex(
            v++,
            { x, -halfHeight, z },
            normal,
            { t, 1.0f }
        );

        SetVertex(
            v++,
            { x, halfHeight, z },
            normal,
            { t, 0.0f }
        );
    }

    int topCenterIndex = v;

    SetVertex(
        v++,
        { 0.0f, halfHeight, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.5f, 0.5f }
    );

    int topRingStart = v;

    for (int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float angle = t * 2.0f * PI;

        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;

        float u = 0.5f + cosf(angle) * 0.5f;
        float vv = 0.5f + sinf(angle) * 0.5f;

        SetVertex(
            v++,
            { x, halfHeight, z },
            { 0.0f, 1.0f, 0.0f },
            { u, vv }
        );
    }

    int bottomCenterIndex = v;

    SetVertex(
        v++,
        { 0.0f, -halfHeight, 0.0f },
        { 0.0f, -1.0f, 0.0f },
        { 0.5f, 0.5f }
    );

    int bottomRingStart = v;

    for (int i = 0; i <= slices; i++)
    {
        float t = (float)i / (float)slices;
        float angle = t * 2.0f * PI;

        float x = cosf(angle) * radius;
        float z = sinf(angle) * radius;

        float u = 0.5f + cosf(angle) * 0.5f;
        float vv = 0.5f - sinf(angle) * 0.5f;

        SetVertex(
            v++,
            { x, -halfHeight, z },
            { 0.0f, -1.0f, 0.0f },
            { u, vv }
        );
    }

    int idx = 0;

    auto AddTriangle = [&](unsigned short a, unsigned short b, unsigned short c)
    {
        mesh.indices[idx++] = a;
        mesh.indices[idx++] = b;
        mesh.indices[idx++] = c;
    };

    for (int i = 0; i < slices; i++)
    {
        unsigned short bottom0 = (unsigned short)(i * 2);
        unsigned short top0 = (unsigned short)(i * 2 + 1);
        unsigned short bottom1 = (unsigned short)((i + 1) * 2);
        unsigned short top1 = (unsigned short)((i + 1) * 2 + 1);

        AddTriangle(bottom0, top0, top1);
        AddTriangle(bottom0, top1, bottom1);
    }

    for (int i = 0; i < slices; i++)
    {
        unsigned short ring0 = (unsigned short)(topRingStart + i);
        unsigned short ring1 = (unsigned short)(topRingStart + i + 1);

        AddTriangle(
            (unsigned short)topCenterIndex,
            ring1,
            ring0
        );
    }

    for (int i = 0; i < slices; i++)
    {
        unsigned short ring0 = (unsigned short)(bottomRingStart + i);
        unsigned short ring1 = (unsigned short)(bottomRingStart + i + 1);

        AddTriangle(
            (unsigned short)bottomCenterIndex,
            ring0,
            ring1
        );
    }

    return mesh;
}
void initModels()
{
    InitLighting();

    Mesh cubeMesh = GenMeshCube(2.0f, 2.0f, 2.0f);
    GenMeshTangents(&cubeMesh);
    cubeModel = LoadModelFromMesh(cubeMesh);
    Cubebounds = GetModelBoundingBox(cubeModel);

    Mesh sphereMesh = GenMeshSphere(1.0f, 32, 32);
    GenMeshTangents(&sphereMesh);
    sphereModel = LoadModelFromMesh(sphereMesh);
    Spherebounds = GetModelBoundingBox(sphereModel);

    Mesh cylinderMesh = GenTexturedCylinder(1.0f, 2.0f, 64);
    GenMeshTangents(&cylinderMesh);
    UploadMesh(&cylinderMesh, false);

    cylinderModel = LoadModelFromMesh(cylinderMesh);
    Cylinderbounds = GetModelBoundingBox(cylinderModel);
    LoadPBRTextures();

    ApplyLightingShader(cubeModel);
    ApplyLightingShader(sphereModel);
    ApplyLightingShader(cylinderModel);


    load();
}
void DrawSceneForShadowMap()
{
    Shader shadowShader = GetShadowShader();

    cubeModel.materials[0].shader = shadowShader;
    sphereModel.materials[0].shader = shadowShader;
    cylinderModel.materials[0].shader = shadowShader;

    for (int i = 0; i < c; i++)
    {
        DrawObjectModel(cubeModel, Cu[i], WHITE);
    }

    for (int i = 0; i < s; i++)
    {
        if (Sp[i].isLight) continue;

        DrawObjectModel(sphereModel, Sp[i], WHITE);
    }

    for (int i = 0; i < y; i++)
    {
        DrawObjectModel(cylinderModel, cy[i], WHITE);
    }

    ApplyLightingShader(cubeModel);
    ApplyLightingShader(sphereModel);
    ApplyLightingShader(cylinderModel);
}

void cube(const Vector3 pos,Color color) {
    if (c < 100) {
        Cu[c].position = pos;
        Cu[c].rotation = { 0.0f, 0.0f, 0.0f };
        Cu[c].scale = { 1.0f, 1.0f, 1.0f };
        Cu[c].color = color;
        Cu[c].isSelected = false;
        Cu[c].material = MATERIAL_WOOD;
        c++;
    }
}

void sphere(const Vector3 pos,Color color){
    if(s<100){
        Sp[s].position = pos;
        Sp[s].rotation = Vector3{0.0f, 0.0f, 0.0f};
        Sp[s].scale = Vector3{1.0f, 1.0f, 1.0f};
        Sp[s].color = color;
        Sp[s].isSelected = false;
        Sp[s].material = MATERIAL_WOOD;
        s++;
    }
}

void cylinder(const Vector3 pos,Color color){
    if(y<100){
        cy[y].position = pos;
        cy[y].rotation = Vector3{0.0f, 0.0f, 0.0f};
        cy[y].scale = Vector3{1.0f, 1.0f, 1.0f};
        cy[y].color = color;
        cy[y].isSelected = false;
        cy[y].material = MATERIAL_WOOD;
        y++;
    }
}

void frameCube() {
    for (int i = 0; i < c; i++) {
        Color renderColor = Cu[i].color;
        renderColor.a = Cu[i].isSelected ? 128 : 255; // changes transparency when objec tis selected
        DrawObjectModel(cubeModel, Cu[i], renderColor);
    }
}

void frameSphere() {
    for (int i = 0; i < s; i++) {

        if (Sp[i].isLight && Sp[i].lightIndex >= 0) {
            SetSceneLightPosition(Sp[i].lightIndex, Sp[i].position);

            // Draw light marker as unlit bright sphere
            DrawSphere(Sp[i].position, Sp[i].scale.x, Sp[i].color);

            if (Sp[i].isSelected) {
                DrawSphereWires(Sp[i].position, Sp[i].scale.x * 1.15f, 16, 16, WHITE);
            }

            continue;
        }

        Color renderColor = Sp[i].color;
        renderColor.a = Sp[i].isSelected ? 128 : 255;

        DrawObjectModel(sphereModel, Sp[i], renderColor);
    }
}

void frameCylinder() {
    for (int i = 0; i < y; i++) {
        Color renderColor = cy[i].color;
        renderColor.a = cy[i].isSelected ? 128 : 255; // changes transparency when objec tis selected
        DrawObjectModel(cylinderModel, cy[i], renderColor);
    }
}
void Unload(void) {
    save();
    UnloadModel(cubeModel);
    UnloadModel(sphereModel);
    UnloadModel(cylinderModel);
    UnloadTextures();
    UnloadLighting();
}
void leftclick(Ray ray){
    
    int closestIdx = -1;
    ObjectType closestType = NONE;
    float closestDist = FLT_MAX;
    for(int i=0; i<c; i++){
            BoundingBox bbox = GetTransformedBounds(Cubebounds, Cu[i].position, Cu[i].scale);
            RayCollision collision = GetRayCollisionBox(ray, bbox);
            if(collision.hit && collision.distance < closestDist){
                closestDist = collision.distance;
                closestIdx = i;
                closestType = CUBE;
            }
        }
    for(int i=0; i<s; i++){
            BoundingBox bbox = GetTransformedBounds(Spherebounds, Sp[i].position, Sp[i].scale);
            RayCollision collision = GetRayCollisionBox(ray, bbox);
            if(collision.hit && collision.distance < closestDist){
                closestDist = collision.distance;
                closestIdx = i;
                closestType = SPHERE;
            }
        }
    for(int i=0; i<y; i++){
            BoundingBox bbox = GetTransformedBounds(Cylinderbounds, cy[i].position, cy[i].scale);
            RayCollision collision = GetRayCollisionBox(ray, bbox);
            if(collision.hit && collision.distance < closestDist){
                closestDist = collision.distance;
                closestIdx = i;
                closestType = CYLINDER;
            }
        }
    bool isShiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        if (!isShiftDown) {
            
            for(int i=0; i<c; i++) Cu[i].isSelected = false;
            for(int i=0; i<s; i++) Sp[i].isSelected = false;
            for(int i=0; i<y; i++) cy[i].isSelected = false;
            totalSelectedCount = 0;
        }

        //Closest Object
        if (closestType != NONE) {
            if (closestType == CUBE)     Cu[closestIdx].isSelected = !Cu[closestIdx].isSelected;
            if (closestType == SPHERE)   Sp[closestIdx].isSelected = !Sp[closestIdx].isSelected;
            if (closestType == CYLINDER) cy[closestIdx].isSelected = !cy[closestIdx].isSelected;
        }

        
        totalSelectedCount = 0;
        for(int i=0; i<c; i++) if (Cu[i].isSelected) totalSelectedCount++;
        for(int i=0; i<s; i++) if (Sp[i].isSelected) totalSelectedCount++;
        for(int i=0; i<y; i++) if (cy[i].isSelected) totalSelectedCount++;
    
}
static Matrix GetObjectTransform(ObjectInstance obj)
{
    Matrix matScale = MatrixScale(
        obj.scale.x,
        obj.scale.y,
        obj.scale.z
    );

    Matrix matRotation = MatrixRotateXYZ({
        DEG2RAD * obj.rotation.x,
        DEG2RAD * obj.rotation.y,
        DEG2RAD * obj.rotation.z
    });

    Matrix matTranslation = MatrixTranslate(
        obj.position.x,
        obj.position.y,
        obj.position.z
    );

    Matrix transform = MatrixMultiply(
        MatrixMultiply(matScale, matRotation),
        matTranslation
    );

    return transform;
}

static void DrawObjectModel(Model& model, ObjectInstance obj, Color color)
{
    ApplyPBRMaterial(model, obj.material);

    Model tempModel = model;
    tempModel.transform = GetObjectTransform(obj);

    DrawModel(tempModel, { 0.0f, 0.0f, 0.0f }, 1.0f, color);
}
bool updateObjectTransformGizmo(Camera3D camera) {
    return UpdateTransformGizmo(
        camera,
        Cu, c,
        Sp, s,
        cy, y
    );
}

void drawObjectTransformGizmo() {
    DrawTransformGizmo(
        Cu, c,
        Sp, s,
        cy, y
    );
}
void lightSphere(const Vector3 pos, Color color)
{
    if (s < 100)
    {
        int lightIndex = CreatePointLight(pos);

        if (lightIndex == -1) return;

        Sp[s].position = pos;
        Sp[s].rotation = Vector3{ 0.0f, 0.0f, 0.0f };
        Sp[s].scale = Vector3{ 0.25f, 0.25f, 0.25f };
        Sp[s].color = color;
        Sp[s].isSelected = false;

        Sp[s].isLight = true;
        Sp[s].lightIndex = lightIndex;

        s++;
    }
}
void deleteobj() {
    for (int i = 0; i < c; i++) {
        if (Cu[i].isSelected) {
            for (int j = i; j < c - 1; j++) {
                Cu[j] = Cu[j + 1];
            }
            c--;
            i--;
        }
    }
    for (int i = 0; i < s; i++) {
    if (Sp[i].isSelected) {

        if (Sp[i].isLight && Sp[i].lightIndex >= 0) {
            int deletedLightIndex = Sp[i].lightIndex;
            DeleteSceneLight(deletedLightIndex);

            // Fix light indices of remaining light spheres
            for (int k = 0; k < s; k++) {
                if (Sp[k].isLight && Sp[k].lightIndex > deletedLightIndex) {
                    Sp[k].lightIndex--;
                }
            }
        }

        for (int j = i; j < s - 1; j++) {
            Sp[j] = Sp[j + 1];
        }

        s--;
        i--;
    }
}
    for (int i = 0; i < y; i++) {
        if (cy[i].isSelected) {
            for (int j = i; j < y - 1; j++) {
                cy[j] = cy[j + 1];
            }
            y--;
            i--;
        }
    }
}