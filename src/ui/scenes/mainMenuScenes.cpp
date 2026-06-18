#include "mainMenuScenes.h"

//Initialise Menu
void menuInit() {
	btnPlay = { btnLeft, btnTop, btnWidth, btnHeight };
	btnOptions = { btnLeft, btnTop + 1 * btnGap, btnWidth, btnHeight };
	btnExit = { btnLeft, btnTop + 2 * btnGap, btnWidth, btnHeight };
}

void menuUnload() {
	// Unload any resources specific to the menu scene here
}

void menuUpdate() {
	if (clickedButton(btnPlay)) sceneManagerChangeScene(sceneId::SCENE_LEARN);
	if (clickedButton(btnOptions)) sceneManagerChangeScene(sceneId::SCENE_OPTIONS);
	if (clickedButton(btnEditor)) sceneManagerChangeScene(sceneId::SCENE_LEARN);

}

void menuDraw() {
	DrawText("DrawCAL", btnLeft, btnTop - 1 * btnGap, 20, BLACK);
	drawButton(btnPlay, "Learn");
	drawButton(btnOptions, "Options");
	drawButton(btnExit, "Exit");
}
