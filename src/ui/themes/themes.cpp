#define RAYGUI_IMPLEMENTATION

#include "themes.h"


// Need to include it here and do the convoluted process because these headers use RAYGUI_MALLOC function
// The compilation process is wonky and it is defined inside raygui_impl.cpp and it has #define RAYGUI_IMPLEMENTATION which gives function defintiions
// It needs to be .cpp because only these does it link the definitions of the functions
// These includes are also inside this .cpp file for the exact reason

#include "style_amber.h"
#include "style_cyber.h"
#include "style_genesis.h"
#include "style_jungle.h"
//#include "amber.rgs.h"
//#include "cyber.rgs.h"
//#include "dark.rgs.h"
#include "style_dark.h"
//#include "genesis.rgs.h"
//#include "jungle.rgs.h"

const char* themeOptions = "Default;Dark;Cyber;Genesis;Jungle;amber";

Vector2 MeasureThemeText(const char* text, float fontSize)
{
    const Font font = GuiGetFont();
    const float spacing = (float)GuiGetStyle(DEFAULT, TEXT_SPACING);
    return MeasureTextEx(font, text, fontSize, spacing);
}

void DrawThemeText(const char* text, float x, float y, float fontSize, Color color)
{
    const Font font = GuiGetFont();
    const float spacing = (float)GuiGetStyle(DEFAULT, TEXT_SPACING);
    DrawTextEx(font, text, { x, y }, fontSize, spacing, color);
}
