// Before executing any function, make sure to check if the function is executable for if the element in scene isn't null

#include "sceneManager.h"

// Ready structures that will take in the function names
sceneFunctions scenes[static_cast<int>(sceneId::SCENE_COUNT)];
sceneFunctions learnScenes[static_cast<int>(learnSceneId::LEARN_COUNT)];

// Define global buttons and state variables (single definition)
Rectangle btnPlay, btnEditor, btnOptions, btnExit;
Rectangle btnFreeDraw, btnGuided, btnTutorial, btnBack;

float btnWidth;
float btnHeight;
float btnLeft;
float btnTop;
float btnGap;

sceneId currentScene = sceneId::SCENE_NONE;
sceneId pendingScene = sceneId::SCENE_NONE;
learnSceneId currentLearnScene = learnSceneId::LEARN_NONE;
learnSceneId pendingLearnScene = learnSceneId::LEARN_NONE;
	
bool sceneInitialized = false; 

void sceneManagerInit() {
	changeButtonResolution();

	// Register scenes
	scenes[static_cast<int>(sceneId::SCENE_MENU)] = { menuInit, menuUpdate, menuDraw, menuUnload };
	scenes[static_cast<int>(sceneId::SCENE_LEARN)] = { learnMenuInit, learnMenuUpdate, learnMenuDraw, learnMenuUnload }; 
	scenes[static_cast<int>(sceneId::SCENE_OPTIONS)] = { NULL, NULL, NULL, NULL }; // Placeholder for Options menu]
	// Register Learn Scenes
	learnScenes[static_cast<int>(learnSceneId::LEARN_MENU)] = { learnMenuInit, learnMenuUpdate, learnMenuDraw, learnMenuUnload }; // pretty sure this is to be executed instead
	learnScenes[static_cast<int>(learnSceneId::LEARN_FREEDRAW)] = { freeDrawInit, freeDrawUpdate, freeDrawDraw, freeDrawUnload };
	learnScenes[static_cast<int>(learnSceneId::LEARN_GUIDED)] = { NULL, NULL, NULL, NULL }; // Placeholder for Guided learning mode
	learnScenes[static_cast<int>(learnSceneId::LEARN_TUTORIAL)] = { NULL, NULL, NULL, NULL }; // Placeholder for Tutorial learning mode

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



static void resolveMainSceneChanges() {
	// Handle scene switching; exiting out of main menu
	if (pendingScene != sceneId::SCENE_NONE) {
		// Update sub level scene
		if (pendingScene == sceneId::SCENE_LEARN)
		{
			pendingLearnScene = learnSceneId::LEARN_MENU;
		}
		// Unload current scene
		if (scenes[static_cast<int>(currentScene)].Unload) scenes[static_cast<int>(currentScene)].Unload(); // Execute only if function is executable
		currentScene = pendingScene;
		pendingScene = sceneId::SCENE_NONE;
		// Initialize new scene
		if (scenes[static_cast<int>(currentScene)].Init) scenes[static_cast<int>(currentScene)].Init();
	}
}

static void resolveLearnSceneChanges() {
	// Handle scene switching; going from learn menu to learn subscene or exiting out of learn mode back to main menu
	if (pendingLearnScene != learnSceneId::LEARN_NONE) {
		// Unload current scene
		if (learnScenes[static_cast<int>(currentLearnScene)].Unload) learnScenes[static_cast<int>(currentLearnScene)].Unload(); // Execute only if function is executable
		currentLearnScene = pendingLearnScene;
		pendingLearnScene = learnSceneId::LEARN_NONE;
		// Initialize new scene
		if (learnScenes[static_cast<int>(currentLearnScene)].Init) learnScenes[static_cast<int>(currentLearnScene)].Init();
	}
}

void sceneManagerUpdate() {
	if (!sceneInitialized) sceneManagerInit(); // First time initialise only
	resolveMainSceneChanges();
	resolveLearnSceneChanges();

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
	




}

void sceneManagerDraw() {
	if (currentScene == sceneId::SCENE_MENU) {
		if (scenes[static_cast<int>(currentScene)].Draw) scenes[static_cast<int>(currentScene)].Draw(); // Should only ever draw the main menu
	}
	else if (currentScene == sceneId::SCENE_LEARN) {
		TraceLog(LOG_INFO, "Drawing Learn Scene");
		if (learnScenes[static_cast<int>(currentLearnScene)].Draw) { 
			learnScenes[static_cast<int>(currentLearnScene)].Draw();
		}
	}
}