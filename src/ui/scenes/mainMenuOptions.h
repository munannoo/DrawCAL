#ifndef mainMenuOptions_H
#define mainMenuOptions_H

#include "sceneManager.h"

#include <string> // For dynamic resolution update, change if better implementation
// dropdown is janky, so obsolete rn, change later maybe

//std::string textFullScreen, textVSync, textResolution;

void optionsMenuInit(); // load textures, initialise objects, set up camera/ui
void optionsMenuUpdate(); // Logical functionality
void optionsMenuDraw(); // Draw on the screen
void optionsMenuUnload(); // Unload textures, remove allocated memory, if any

void optionsGraphicsInit();
void optionsGraphicsUpdate();
void optionsGraphicsDraw();
void optionsGraphicsUnload();

void optionsControlsInit();
void optionsControlsUpdate();
void optionsControlsDraw();
void optionsControlsUnload();

void optionsInterfaceInit();
void optionsInterfaceUpdate();
void optionsInterfaceDraw();
void optionsInterfaceUnload();

void changeResolution();

void changeTheme();

#endif 