#ifndef FreeDrawMode_H
#define FreeDrawMode_H

#include "raylib.h"
#include "main.h"
#include "features/shadings/textures.h"
#include "features/camera/CameraController.h"
#include "rendering/renderer.h"
#include "input/InputHandler.h"
#include "objects/object.h"
#include "features/manipulation/Transform.h"
#include "features/learning/FreeDrawMode.h"
#include "ui/scenes/sceneManager.h"
#include "features/shadings/lighting.h"
#include "raygui.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// selected view (0=Front,1=Top,2=Left,3=Right)
// Put free draw mode specific variables/Textures here, they will be initialised in freeDrawInit and unloaded in freeDrawUnload
static struct freeDrawState {
	Rectangle drawArea;
	int check;
	bool mouseButtonPressed;
	bool initiliased; // if the init function has been called
	bool dropdownEditmode; 
	// View dropdown state (top-right)
	cameraView currentViewIndex, lastViewIndex;
	bool viewDropdownOpen; 
	bool cameraLocked;   // when true, camera controller movement is disabled (only zoom allowed)
	bool helpTip; // Press F1
	bool splitScreenEnabled;
	bool cameraPanelOpen;
	bool propertiesPanelOpen;
	bool workspacePanelOpen;
	std::vector<ViewportSlot> activeViews;
	bool usingGizmo;
} freeDrawState;



void freeDrawInit();
void freeDrawUpdate();
void freeDrawDraw();
void freeDrawUnload();
#endif // FreeDrawMode_H