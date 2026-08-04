#include "mainMenuScenes.h"
#include "UiStyle.h"

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

static GuiWindowFileDialogState fileDialogState;
static bool fileDialogInitialized = false;
static bool fileDialogIsSaveMode = false;

static void OpenSaveDialog()
{
    if (!fileDialogInitialized)
    {
        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
        fileDialogInitialized = true;
    }
    strcpy(fileDialogState.filterExt, ".json");
    fileDialogState.saveFileMode = true;
    fileDialogState.windowActive = true;
    fileDialogIsSaveMode = true;
}

static void OpenLoadDialog()
{
    if (!fileDialogInitialized)
    {
        fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
        fileDialogInitialized = true;
    }
    strcpy(fileDialogState.filterExt, ".json");
    fileDialogState.saveFileMode = false;
    fileDialogState.windowActive = true;
    fileDialogIsSaveMode = false;
}

//Initialise Menu
void menuInit() {


    changeButtonResolution();
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

    UiStyle::DrawSceneHeader("DrawCAL", "Create. Learn. Design.", headerTop);

    bool dialogOpen = fileDialogState.windowActive;

    if (dialogOpen) GuiDisable();

    if (GuiButton(btnPlay, "Learn")) sceneManagerChangeScene(sceneId::SCENE_LEARN);
    if (GuiButton(btnSave, "Save Scene")) OpenSaveDialog();
    if (GuiButton(btnLoad, "Load Scene")) OpenLoadDialog();
    if (GuiButton(btnOptions, "Options")) sceneManagerChangeScene(sceneId::SCENE_OPTIONS);

    if (GuiButton(btnExit, "Exit")) {
        menuUnload();
        exitWindow = true;
    }

    if (dialogOpen) GuiEnable();

    if (fileDialogState.windowActive)
    {
        GuiWindowFileDialog(&fileDialogState);

        if (fileDialogState.SelectFilePressed)
        {
            std::string fullPath = std::string(fileDialogState.dirPathText) + "/" + fileDialogState.fileNameText;

            if (fileDialogIsSaveMode && !IsFileExtension(fullPath.c_str(), ".json"))
            {
                fullPath += ".json";
            }

            bool ok = fileDialogIsSaveMode ? saveScene(fullPath) : loadScene(fullPath);
            if (!ok)
            {
                TraceLog(LOG_WARNING, "Scene %s failed: %s",
                    fileDialogIsSaveMode ? "save" : "load", fullPath.c_str());
            }

            fileDialogState.SelectFilePressed = false;
            fileDialogState.windowActive = false;
        }
    }
}