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

}

void menuDraw() {
	DrawText("DrawCAL", btnLeft, btnTop - 1 * btnGap, 20, BLACK);
	if (GuiButton(btnPlay, "Learn")) sceneManagerChangeScene(sceneId::SCENE_LEARN);
	if (GuiButton(btnOptions, "Options")) { sceneManagerChangeScene(sceneId::SCENE_OPTIONS); TraceLog(LOG_INFO, "Clicked %d", pendingScene); }
	if (GuiButton(btnExit, "Exit")) {
		menuUnload();
		exitWindow = true;
	}
}
