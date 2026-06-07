#include "buttons.h"
//void createObj(Vector3 pos)
//{
//    // Implementation for creating an object at the specified position
//}

bool button(Rectangle r, const char* label)
{
    Vector2 m = GetMousePosition();
	bool hovered = CheckCollisionPointRec(m, r); // Check if mouse is over the button
    DrawRectangleRec(r, hovered ? LIGHTGRAY : GRAY); // Change colour if hovered
    DrawText(label, (int)(r.x + 10), (int)(r.y + 8), 20, BLACK);
	return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON); // Return true if clicked
}