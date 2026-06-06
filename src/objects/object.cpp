#include "object.h"
#include "features/manipulation/Transform.h"
#include "raylib.h"
#include <float.h>
#include "raymath.h"
#include "rlgl.h"

static Model cubeModel;
static Model sphereModel;
static Model cylinderModel;

static BoundingBox Cubebounds;
static BoundingBox Spherebounds;
static BoundingBox Cylinderbounds;

static ObjectInstance Cu[100];
static ObjectInstance Sp[100];
static ObjectInstance cy[100];

// forward declaration to allow calls before definition
static void DrawObjectModel(Model model, ObjectInstance obj, Color color);

enum ObjectType { NONE, CUBE, SPHERE, CYLINDER };
static ObjectType selectedType = NONE;
static int c = 0;
static int s = 0;
static int y = 0;
static int totalSelectedCount = 0;

void initModels(){
    Mesh cubeMesh = GenMeshCube(2.0f, 2.0f, 2.0f);
    cubeModel = LoadModelFromMesh(cubeMesh);
    Cubebounds = GetModelBoundingBox(cubeModel);
    Mesh sphereMesh = GenMeshSphere(1.0f, 16, 16);
    sphereModel = LoadModelFromMesh(sphereMesh);
    Spherebounds = GetModelBoundingBox(sphereModel);
    Mesh cylinderMesh = GenMeshCylinder(1.0f, 2.0f, 16);
    cylinderModel = LoadModelFromMesh(cylinderMesh);
    Cylinderbounds = GetModelBoundingBox(cylinderModel);
}


void cube(const Vector3 pos) {
    if (c < 100) {
        Cu[c].position = pos;
        Cu[c].rotation = { 0.0f, 0.0f, 0.0f };
        Cu[c].scale = { 1.0f, 1.0f, 1.0f };
        Cu[c].color = GRAY;
        Cu[c].isSelected = false;
        c++;
    }
}

void sphere(const Vector3 pos){
    if(s<100){
        Sp[s].position = pos;
        Sp[s].rotation = Vector3{0.0f, 0.0f, 0.0f};
        Sp[s].scale = Vector3{1.0f, 1.0f, 1.0f};
        Sp[s].color = GRAY;
        Sp[s].isSelected = false;
        s++;
    }
}

void cylinder(const Vector3 pos){
    if(y<100){
        cy[y].position = pos;
        cy[y].rotation = Vector3{0.0f, 0.0f, 0.0f};
        cy[y].scale = Vector3{1.0f, 1.0f, 1.0f};
        cy[y].color = GRAY;
        cy[y].isSelected = false;
        y++;
    }
}

void frameCube() {
    for (int i = 0; i < c; i++) {
        Color renderColor = Cu[i].isSelected ? BLUE : Cu[i].color;
        DrawObjectModel(cubeModel, Cu[i], renderColor);
    }
}

void frameSphere() {
    for (int i = 0; i < s; i++) {
        Color renderColor = Sp[i].isSelected ? BLUE : Sp[i].color;
        DrawObjectModel(sphereModel, Sp[i], renderColor);
    }
}

void frameCylinder() {
    for (int i = 0; i < y; i++) {
        Color renderColor = cy[i].isSelected ? BLUE : cy[i].color;
        DrawObjectModel(cylinderModel, cy[i], renderColor);
    }
}
void Unload(void) {
    UnloadModel(cubeModel);
    UnloadModel(sphereModel);
    UnloadModel(cylinderModel);
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
static void DrawObjectModel(Model model, ObjectInstance obj, Color color) {
    rlPushMatrix();

        rlTranslatef(obj.position.x, obj.position.y, obj.position.z);

        rlRotatef(obj.rotation.x, 1.0f, 0.0f, 0.0f);
        rlRotatef(obj.rotation.y, 0.0f, 1.0f, 0.0f);
        rlRotatef(obj.rotation.z, 0.0f, 0.0f, 1.0f);

        rlScalef(obj.scale.x, obj.scale.y, obj.scale.z);

        DrawModel(model, { 0.0f, 0.0f, 0.0f }, 1.0f, color);

    rlPopMatrix();
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