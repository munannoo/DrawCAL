#include "learnMenuScenes.h"
#include "UiStyle.h"

//Initialise Learn Scenes
void learnMenuInit() {
	btnFreeDraw = { btnLeft, btnTop, btnWidth, btnHeight };
	btnGuided = {btnLeft, btnTop + btnGap, btnWidth, btnHeight};
	btnBack = {btnLeft, btnTop + 2 * btnGap, btnWidth, btnHeight};
}

void learnMenuUnload() {
	// Unload any resources specific to the menu scene here
}

void learnMenuUpdate() {

}

void learnMenuDraw() {
	UiStyle::LoadMinimalStyle();
	UiStyle::DrawBackground();
	UiStyle::DrawSceneHeader("Learn", "Choose your path.", btnTop - 68.0f, btnTop - 28.0f);

	if (GuiButton(btnFreeDraw, "Free Draw")) {
		SetGuidedWorkspace(false);
		sceneManagerChangeScene(learnSceneId::LEARN_FREEDRAW);
	}
	if (GuiButton(btnGuided, "Guided")) sceneManagerChangeScene(learnSceneId::LEARN_GUIDED);

	if (GuiButton(btnBack, "Back")) {
		pendingScene = sceneId::SCENE_MENU;
		currentLearnScene = learnSceneId::LEARN_NONE;
	}
}
