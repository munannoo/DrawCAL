// Before executing any function, make sure to check if the function is executable for if the element in scene isn't null

#include "sceneManager.h"

// Ready structures that will take in the function names
sceneFunctions scenes[static_cast<int>(sceneId::SCENE_COUNT)];
sceneFunctions learnScenes[static_cast<int>(learnSceneId::LEARN_COUNT)];
sceneFunctions optionScenes[static_cast<int>(optionSceneId::OPTIONS_COUNT)];
// Define global buttons and state variables (single definition)
Rectangle btnPlay, btnEditor, btnOptions, btnExit, btnSave, btnLoad;
Rectangle btnFreeDraw, btnGuided, btnTutorial, btnBack;
Rectangle btnGraphics, btnControls, btnInterface;
Rectangle btnResolution, btnVSync, btnFullScreen;
Rectangle btnUIScale, btnTheme, btnFontSize;

float btnWidth;
float btnHeight;
float btnLeft;
float btnTop;
float btnGap;

sceneId currentScene = sceneId::SCENE_NONE;
sceneId pendingScene = sceneId::SCENE_NONE;
learnSceneId currentLearnScene = learnSceneId::LEARN_NONE;
learnSceneId pendingLearnScene = learnSceneId::LEARN_NONE;
optionSceneId currentOptionScene = optionSceneId::OPTIONS_NONE;
optionSceneId pendingOptionScene = optionSceneId::OPTIONS_NONE;


bool sceneInitialized = false; 

void sceneManagerInit() {
	initModels(); // Initialize the Models, only needs to be called once
	InitTransformGizmo();
	initgridShader();

	changeButtonResolution();
	GuiLoadStyleDefault();
	// Register scenes
	scenes[static_cast<int>(sceneId::SCENE_MENU)] = { menuInit, menuUpdate, menuDraw, menuUnload };
	scenes[static_cast<int>(sceneId::SCENE_LEARN)] = { learnMenuInit, learnMenuUpdate, learnMenuDraw, learnMenuUnload }; 
	scenes[static_cast<int>(sceneId::SCENE_OPTIONS)] = { optionsMenuDraw, optionsMenuUpdate, optionsMenuDraw, optionsMenuUnload }; // Placeholder for Options menu]
	// Register Learn Scenes
	learnScenes[static_cast<int>(learnSceneId::LEARN_MENU)] = { learnMenuInit, learnMenuUpdate, learnMenuDraw, learnMenuUnload }; // pretty sure this is to be executed instead
	learnScenes[static_cast<int>(learnSceneId::LEARN_FREEDRAW)] = { freeDrawInit, freeDrawUpdate, freeDrawDraw, freeDrawUnload };
	learnScenes[static_cast<int>(learnSceneId::LEARN_GUIDED)] = { NULL, NULL, NULL, NULL }; // Placeholder for Guided learning mode
	learnScenes[static_cast<int>(learnSceneId::LEARN_TUTORIAL)] = { NULL, NULL, NULL, NULL }; // Placeholder for Tutorial learning mode
	// Register Options Scenes
	optionScenes[static_cast<int>(optionSceneId::OPTIONS_MENU)] = { optionsMenuInit, optionsMenuUpdate, optionsMenuDraw, optionsMenuUnload };
	optionScenes[static_cast<int>(optionSceneId::OPTIONS_CONTROLS)] = { optionsControlsInit, optionsControlsUpdate, optionsControlsDraw, optionsControlsUnload };
	optionScenes[static_cast<int>(optionSceneId::OPTIONS_GRAPHICS)] = { optionsGraphicsInit, optionsGraphicsUpdate, optionsGraphicsDraw, optionsGraphicsUnload };
	optionScenes[static_cast<int>(optionSceneId::OPTIONS_INTERFACE)] = { optionsInterfaceInit, optionsInterfaceUpdate, optionsInterfaceDraw, optionsInterfaceUnload };
	// Initialise current scene
	currentScene = sceneId::SCENE_MENU;
	if (scenes[static_cast<int>(currentScene)].Init) scenes[static_cast<int>(currentScene)].Init();
	sceneInitialized = true;
}

// Maybe can template this but idk how

void sceneManagerChangeScene(sceneId newScene) {
	pendingScene = newScene;
}

void sceneManagerChangeScene(learnSceneId newLearnScene) {
	pendingLearnScene = newLearnScene;
}

void sceneManagerChangeScene(optionSceneId newOptionScene) {
	pendingOptionScene = newOptionScene;
}

// Update Scene Logics

static void resolveMainSceneChanges() {
	// Handle scene switching; exiting out of main menu
	if (pendingScene != sceneId::SCENE_NONE) {
		// Update sub level scene
		if (pendingScene == sceneId::SCENE_LEARN)
		{
			pendingLearnScene = learnSceneId::LEARN_MENU;
		}
		else if (pendingScene == sceneId::SCENE_OPTIONS) {
			pendingOptionScene = optionSceneId::OPTIONS_MENU;
		}

		// Unload current scene
		if (scenes[static_cast<int>(currentScene)].Unload) scenes[static_cast<int>(currentScene)].Unload(); // Execute only if function is executable
		currentScene = pendingScene;
		pendingScene = sceneId::SCENE_NONE;
		// Initialize new scene
		if (scenes[static_cast<int>(currentScene)].Init) scenes[static_cast<int>(currentScene)].Init();
	}
}

static void resolveLearnSceneChanges()
{
    if (pendingLearnScene != learnSceneId::LEARN_NONE)
    {
        if (currentLearnScene != learnSceneId::LEARN_NONE)
        {
            if (learnScenes[static_cast<int>(currentLearnScene)].Unload)
            {
                learnScenes[static_cast<int>(currentLearnScene)].Unload();
            }
        }

        currentLearnScene = pendingLearnScene;
        pendingLearnScene = learnSceneId::LEARN_NONE;

        if (learnScenes[static_cast<int>(currentLearnScene)].Init)
        {
            learnScenes[static_cast<int>(currentLearnScene)].Init();
        }
    }
}

static void resolveOptionSceneChanges() {
	// Handle scene switching; going from learn menu to learn subscene or exiting out of learn mode back to main menu
	if (pendingOptionScene != optionSceneId::OPTIONS_NONE) {
		TraceLog(LOG_INFO, "123");
		// Unload current scene
		if (optionScenes[static_cast<int>(currentOptionScene)].Unload) optionScenes[static_cast<int>(currentOptionScene)].Unload();
		currentOptionScene = pendingOptionScene;
		pendingOptionScene = optionSceneId::OPTIONS_NONE;
		// Initialize new scene
		if (optionScenes[static_cast<int>(currentOptionScene)].Init) optionScenes[static_cast<int>(currentOptionScene)].Init();
	}
}

void sceneManagerUpdate() {
	if (!sceneInitialized) sceneManagerInit(); // First time initialise only
	resolveMainSceneChanges();
	resolveLearnSceneChanges();
	resolveOptionSceneChanges();

	if (currentScene == sceneId::SCENE_MENU) {
		if (scenes[static_cast<int>(currentScene)].Update) scenes[static_cast<int>(currentScene)].Update(); // Update the main menu if we are under the main menu
	}

	// Under Learn Scene, we don't use any flags for if we are under the Learn Scene, our flag is currentScene itself
	if (currentScene == sceneId::SCENE_LEARN) {

		if (currentLearnScene != learnSceneId::LEARN_NONE) // Update only if we select something
		{
			if (learnScenes[static_cast<int>(currentLearnScene)].Update) learnScenes[static_cast<int>(currentLearnScene)].Update();
		}
		else {
			if (scenes[static_cast<int>(currentScene)].Update) scenes[static_cast<int>(currentScene)].Update(); // Update the learn menu if we haven't selected a learn subscene
		}
	}
	else if (currentScene == sceneId::SCENE_OPTIONS)
	{
		if (currentOptionScene != optionSceneId::OPTIONS_NONE)
		{
			if (optionScenes[static_cast<int>(currentOptionScene)].Update) optionScenes[static_cast<int>(currentOptionScene)].Update();
		}
		else {
			if (scenes[static_cast<int>(currentScene)].Update) scenes[static_cast<int>(currentScene)].Update(); 
		}
	}
	




}

void sceneManagerDraw() {
	if (currentScene == sceneId::SCENE_MENU) {
		if (scenes[static_cast<int>(currentScene)].Draw) scenes[static_cast<int>(currentScene)].Draw(); // Should only ever draw the main menu
	}
	else if (currentScene == sceneId::SCENE_LEARN) {
		if (learnScenes[static_cast<int>(currentLearnScene)].Draw) { 
			learnScenes[static_cast<int>(currentLearnScene)].Draw();
		}
	}
	else if (currentScene == sceneId::SCENE_OPTIONS) {
		//TraceLog(LOG_INFO, "Drawing Learn Scene %d" ,currentOptionScene);
		if (optionScenes[static_cast<int>(currentOptionScene)].Draw)
		{
			optionScenes[static_cast<int>(currentOptionScene)].Draw();
		}
	}
}