#ifndef mainMenuOptions_H
#define mainMenuOptions_H

#include "sceneManager.h"

void optionsMenuInit(); // load textures, initialise objects, set up camera/ui
void optionsMenuUpdate(); // Logical functionality
void optionsMenuDraw(); // Draw on the screen
void optionsMenuUnload(); // Unload textures, remove allocated memory, if any

#endif 