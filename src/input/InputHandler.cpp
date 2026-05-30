#include "InputHandler.h"
#include "../ui/widgets/buttons.h"
#include  <raygui.h>
#include <objects/object.h>
static Vector2 clickPos = { 0.0f, 0.0f };
static Vector3 objPosn = { 0.0f, 0.0f, 0.0f };
void rightclick(int& r,Camera3D& camera)
{   
    
    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
        r=1;
        clickPos = GetMousePosition();
    }
    Ray ray = GetMouseRay(clickPos, camera);
    float planeSize = 10000.0f;
    Vector3 p1 = { -planeSize, 0.0f, -planeSize };
    Vector3 p2 = {  planeSize, 0.0f, -planeSize };
    Vector3 p3 = {  planeSize, 0.0f,  planeSize };
    Vector3 p4 = { -planeSize, 0.0f,  planeSize };
    RayCollision collision = GetRayCollisionQuad(ray, p1, p2, p3, p4);
    if(collision.hit){
        objPosn = collision.point;
        
    }
    objPosn.y = 0.0f;
    // cout << "Hit at: " << objPosn.x << ", " << objPosn.y << ", " << objPosn.z << endl;
    if(r){
        DrawRectangle( clickPos.x,  clickPos.y, 150, 150, GRAY); 
        Rectangle b1 = { clickPos.x, clickPos.y, 150, 50 };
        Rectangle b2 = { clickPos.x, clickPos.y+50, 150, 50 };
        Rectangle b3 = { clickPos.x, clickPos.y+100, 150, 50 };
        if(GuiButton(b1, "Insert Cube")){
            r=0;
            DrawRectangle( clickPos.x,  clickPos.y, 150, 50, DARKGRAY); 
            Cube(objPosn);
        }
        if(GuiButton(b2, "Insert Sphere")){
            r=0;
            DrawRectangle( clickPos.x,  clickPos.y, 150, 50, DARKGRAY); 
            Sphere(objPosn);
        }
        if(GuiButton(b3, "Insert Cylinder")){
            r=0;
            DrawRectangle( clickPos.x,  clickPos.y, 150, 50, DARKGRAY); 
            Cylinder(objPosn);
        }
    }
    
}