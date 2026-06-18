#include "learnMenuScenes.h"

//Initialise Learn Scenes
void learnMenuInit() {
	TraceLog(LOG_INFO, "Initializing Learn Menu Scene");
	btnFreeDraw = { btnLeft, btnTop, btnWidth, btnHeight };
	btnGuided = {btnLeft, btnTop + btnGap, btnWidth, btnHeight};
	btnTutorial = {btnLeft, btnTop + 2 * btnGap, btnWidth, btnHeight};
	btnBack = {btnLeft, btnTop + 3 * btnGap, btnWidth, btnHeight};
}

void learnMenuUnload() {
	// Unload any resources specific to the menu scene here
}

void learnMenuUpdate() {
	TraceLog(LOG_INFO, "upppp Learn Menu Scene");

	if (clickedButton(btnFreeDraw)) sceneManagerChangeScene(learnSceneId::LEARN_FREEDRAW);
	if (clickedButton(btnGuided)) sceneManagerChangeScene(learnSceneId::LEARN_GUIDED);
	if (clickedButton(btnTutorial)) sceneManagerChangeScene(learnSceneId::LEARN_TUTORIAL);
	if (clickedButton(btnBack)) sceneManagerChangeScene(learnSceneId::LEARN_EXIT);
}

void learnMenuDraw() {
	TraceLog(LOG_INFO, "draw Learn Menu Scene");

	//if (currentLearnScene != learnSceneId::LEARN_MENU)
	//	return;
	// Scene Changers (use learn-specific helper to request learn subscenes)
	DrawText("LEARN", btnLeft, btnTop - 1 * btnGap, 20, BLACK);
	drawButton(btnFreeDraw, "Free Draw");
	drawButton(btnGuided, "Guided");
	drawButton(btnTutorial, "Tutorial");
	drawButton(btnBack, "Back");
}

