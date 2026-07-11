#include "mainMenuScenes.h"
#include "UiStyle.h"

//Initialise Menu
void menuInit() {
    float screenW = static_cast<float>(GetScreenWidth());
    float screenH = static_cast<float>(GetScreenHeight());

    btnWidth = screenW * 0.26f;
    btnHeight = screenH * 0.075f;
    btnGap = btnHeight * 1.5f;
    btnLeft = (screenW - btnWidth) * 0.5f;

    const float totalButtonHeight = btnHeight * 5.0f + btnGap * 4.0f;
    const float contentHeight = totalButtonHeight + 100.0f;
    btnTop = (screenH - contentHeight) * 0.5f + 80.0f;

    btnPlay = { btnLeft, btnTop, btnWidth, btnHeight };
    btnSave = { btnLeft, btnTop + btnGap, btnWidth, btnHeight };
    btnLoad = { btnLeft, btnTop + 2 * btnGap, btnWidth, btnHeight };
    btnOptions = { btnLeft, btnTop + 3 * btnGap, btnWidth, btnHeight };
    btnExit = { btnLeft, btnTop + 4 * btnGap, btnWidth, btnHeight };
}

void menuUnload() {
    // Unload any resources specific to the menu scene here
}

void menuUpdate() {

}

void menuDraw() {
    UiStyle::LoadMinimalStyle();
    UiStyle::DrawBackground();

    UiStyle::DrawSceneHeader("DrawCAL", "Create. Learn. Design.", btnTop - 68.0f, btnTop - 28.0f);

    if (GuiButton(btnPlay, "Learn")) sceneManagerChangeScene(sceneId::SCENE_LEARN);
    if (GuiButton(btnSave, "Save")) save();
    if (GuiButton(btnLoad, "Load")) load();
    if (GuiButton(btnOptions, "Options")) { sceneManagerChangeScene(sceneId::SCENE_OPTIONS); TraceLog(LOG_INFO, "Clicked %d", pendingScene); }
    if (GuiButton(btnExit, "Exit")) {
        menuUnload();
        exitWindow = true;
    }
}
