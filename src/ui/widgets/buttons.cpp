#include "buttons.h"
//void createObj(Vector3 pos)
//{
//    // Implementation for creating an object at the specified position
//}

void drawButton(Rectangle r, const char* label)
{
	bool hovered = CheckCollisionPointRec(GetMousePosition(), r); // Check if mouse is over the button
	DrawRectangleRec(r, hovered ? LIGHTGRAY : GRAY); // Change colour if hovered
	DrawText(label, (int)(r.x + 10), (int)(r.y + 8), 20, BLACK);
}

bool clickedButton(Rectangle r)
{
	bool hovered = CheckCollisionPointRec(GetMousePosition(), r); // Check if mouse is over the button
	return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON); // Return true if clicked
}