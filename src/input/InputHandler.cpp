#include "InputHandler.h"
#include "../ui/widgets/buttons.h"
#include  <raygui.h>
#include <objects/object.h>
static Vector2 clickPos = { 0.0f, 0.0f };
void rightclick(int& r)
{   
    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
        r=1;
        clickPos = GetMousePosition();
    }
    if(r){
        DrawRectangle( clickPos.x,  clickPos.y, 150, 150, GRAY); 
        Rectangle b1 = { clickPos.x, clickPos.y, 150, 50 };
        if(GuiButton(b1, "Insert Cube")){
            r=0;
            DrawRectangle( clickPos.x,  clickPos.y, 150, 50, DARKGRAY); 
            Cube(Vector3{clickPos.x, clickPos.y, 0});
        }
        Rectangle b2 = { clickPos.x, clickPos.y+50, 150, 50 };
        if(GuiButton(b2, "Insert Sphere")){
            r=0;
            DrawRectangle( clickPos.x,  clickPos.y, 150, 50, DARKGRAY); 
            Sphere(Vector3{clickPos.x, clickPos.y, 0});
        }
        Rectangle b3 = { clickPos.x, clickPos.y+100, 150, 50 };
        if(GuiButton(b3, "Insert Cylinder")){
            r=0;
            DrawRectangle( clickPos.x,  clickPos.y, 150, 50, DARKGRAY); 
            Cylinder(Vector3{clickPos.x, clickPos.y, 0});
        }
    }
    else{
        DrawRectangle( clickPos.x,  clickPos.y, 100, 50, GRAY); 
    }
}