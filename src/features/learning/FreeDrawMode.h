#ifndef FreeDrawMode_H
#define FreeDrawMode_H
#include "raylib.h"
#include "main.h"
#include "features/shadings/textures.h"
#include "features/camera/CameraController.h"
#include "rendering/renderer.h"
#include "input/InputHandler.h"
#include "ui/panels/toolbar.h"
#include "ui/widgets/buttons.h"
#include "objects/object.h"
#include "features/manipulation/Transform.h"
#include "features/learning/FreeDrawMode.h"
#include "ui/scenes/sceneManager.h"

// Put free draw mode specific variables/Textures here, they will be initialised in freeDrawInit and unloaded in freeDrawUnload
static struct freeDrawState {
	Camera3D camera;
	Rectangle drawArea;
	int check;
	bool mouseButtonPressed;
	bool initiliased;
	bool dropdownEditmode;
} freeDrawState;

void freeDrawInit();
void freeDrawUpdate();
void freeDrawDraw();
void freeDrawUnload();

#endif // FreeDrawMode_H