#ifndef mainMenuScenes_H
#define mainMenuScenes_H

#include "sceneManager.h"


void menuInit(); // load textures, initialise objects, set up camera/ui
void menuUnload(); // Unload textures, remove allocated memory, if any
void menuUpdate(); // Logical functionality
void menuDraw(); // Draw on the screen


#endif 