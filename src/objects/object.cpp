#include "object.h"
#include "features/manipulation/Transform.h"
static Model cubeModel;
static Model sphereModel;
static Model cylinderModel;

static ObjectInstance Cu[100];
static ObjectInstance Sp[100];
static ObjectInstance cy[100];

static int c = 0;
static int s = 0;
static int y = 0;

void initModels(){
    Mesh cubeMesh = GenMeshCube(2.0f, 2.0f, 2.0f);
    cubeModel = LoadModelFromMesh(cubeMesh);
    Mesh sphereMesh = GenMeshSphere(1.0f, 16, 16);
    sphereModel = LoadModelFromMesh(sphereMesh);
    Mesh cylinderMesh = GenMeshCylinder(1.0f, 2.0f, 16);
    cylinderModel = LoadModelFromMesh(cylinderMesh);

}
void cube(const Vector3 pos){
    
    Cu[c].position = pos;
    Cu[c].rotationAxis = Vector3{0.0f, 1.0f, 0.0f};
    Cu[c].rotationAngle = 0.0f;
    Cu[c].scale = Vector3{1.0f, 1.0f, 1.0f};
    Cu[c].color = GRAY;
    
    c++;
    
}

void sphere(const Vector3 pos){
    if(s<100){
        Sp[s].position = pos;
        Sp[s].rotationAxis = Vector3{0.0f, 1.0f, 0.0f};
        Sp[s].rotationAngle = 0.0f;
        Sp[s].scale = Vector3{1.0f, 1.0f, 1.0f};
        Sp[s].color = GRAY;
        s++;
    }
}

void cylinder(const Vector3 pos){
    if(y<100){
        cy[y].position = pos;
        cy[y].rotationAxis = Vector3{0.0f, 1.0f, 0.0f};
        cy[y].rotationAngle = 0.0f;
        cy[y].scale = Vector3{1.0f, 1.0f, 1.0f};
        cy[y].color = GRAY;
        y++;
    }
}

void frameCube(){
    for(int i=0; i<c; i++){
        DrawModelEx(cubeModel, Cu[i].position, Cu[i].rotationAxis, Cu[i].rotationAngle, Cu[i].scale, Cu[i].color);
    }
}

void frameSphere(){
    for(int i=0; i<s; i++){
        DrawModelEx(sphereModel, Sp[i].position, Sp[i].rotationAxis, Sp[i].rotationAngle, Sp[i].scale, Sp[i].color);
    }
}

void frameCylinder(){
    for(int i=0; i<y; i++){
        DrawModelEx(cylinderModel, cy[i].position, cy[i].rotationAxis, cy[i].rotationAngle, cy[i].scale, cy[i].color);
    }
}
void Unload(void) {
    UnloadModel(cubeModel);
    UnloadModel(sphereModel);
    UnloadModel(cylinderModel);
}
