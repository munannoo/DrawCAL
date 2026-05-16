#include "InputHandler.h"
#include "../ui/widgets/buttons.h"
void rightclick()
{
    Vector2 mousePos = GetMousePosition();
    DrawText("Right Clicked!", mousePos.x, mousePos.y, 20, WHITE);
    DrawRectangle( mousePos.x,  mousePos.y, 100, 50, GRAY); 
}