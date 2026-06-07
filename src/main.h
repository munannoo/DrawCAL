#ifndef main_H
#define main_H

#include "raylib.h"
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

extern int currentResIndex;
extern int lastResIndex;

//enum Scene_Learn
//{
//	LEARN_NONE = 0,
//	LEARN_FREEDRAW,
//	LEARN_GUIDED,
//	LEARN_TUTORIAL
//};
//
//enum Scene_State
//{
//	SCENE_NONE = 0,
//	SCENE_LEARN_MODE,
//	SCENE_CONTROLS_MODE
//};

#endif // main_h

