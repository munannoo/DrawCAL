#include "object.h"
static Vector3 Cu[100];
static Vector3 Sp[100];
static Vector3 cy[100];

static int c = 0;
static int s = 0;
static int y = 0;

void cube(const Vector3 pos){
    if(c<100){
        Cu[c] = pos;
        c++;
    }
}

void sphere(const Vector3 pos){
    if(s<100){
        Sp[s] = pos;
        s++;
    }
}

void cylinder(const Vector3 pos){
    if(y<100){
        cy[y] = pos;
        y++;
    }
}

void frameCube(){
    for(int i=0; i<c; i++){
        DrawCube(Cu[i], 2.0f, 2.0f, 2.0f, DARKGRAY);
    }
}

void frameSphere(){
    for(int i=0; i<s; i++){
        DrawSphere(Sp[i], 2.0f, DARKGRAY);
    }
}

void frameCylinder(){
    for(int i=0; i<y; i++){
        DrawCylinder(cy[i], 1.0f, 1.0f, 2.0f, 16, DARKGRAY);
    }
}
