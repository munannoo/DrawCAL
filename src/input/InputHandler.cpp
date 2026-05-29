#include "InputHandler.h"
#include "../ui/widgets/buttons.h"
static Vector2 clickPos = { 0.0f, 0.0f };
void rightclick(int& r)
{   
    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
        r=1;
        clickPos = GetMousePosition();
    }
    if(r==1){
        DrawRectangle( clickPos.x,  clickPos.y, 150, 50, GRAY); 
        
    }
    else{
        DrawRectangle( clickPos.x,  clickPos.y, 100, 50, GRAY); 
    }
}