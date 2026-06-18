#ifndef learnMenuScenes_H
#define learnMenuScenes_H

#include "sceneManager.h"


void learnMenuInit(); // load textures, initialise objects, set up camera/ui
void learnMenuUnload(); // Unload textures, remove allocated memory, if any
void learnMenuUpdate(); // Logical functionality
void learnMenuDraw(); // Draw on the screen


#endif 