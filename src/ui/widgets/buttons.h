#ifndef button_h
#define button_h

#include "raylib.h"

void drawButton(Rectangle r, const char* label);

bool clickedButton(Rectangle r);

#endif // button_h
