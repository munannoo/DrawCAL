#pragma once

#include "raylib.h"

enum GuidedButtonIcon
{
    ICON_CUBE,
    ICON_SPHERE,
    ICON_CYLINDER,
    ICON_IMPORT
};

void GuidedModeInit();
void GuidedModeUpdate();
void GuidedModeDraw();
void GuidedModeUnload();

void DrawRoundedButton(Rectangle bounds,
                       Color color,
                       const char* text,
                       GuidedButtonIcon icon);