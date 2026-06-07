#include "sceneManager.h"
#include <string.h>

static Rectangle bPlay, bEditor, bOptions, bQuit;
static Rectangle bFreeDraw, bGuided, bTutorial, bBack;

// Initliase variables
static scene scenes[sceneCount];
static scene learnScenes[learnSceneCount];
static sceneId currentScene = SCENE_MENU;
static sceneId pendingScene = SCENE_NONE;
static learnSceneId pendingLearnScene = LEARN_NONE;
static learnSceneId currentLearnScene = LEARN_MENU;
static bool started = false;

static bool sceneLoaded[sceneCount] = { false }; // Track loaded state of each scene
static bool learnLoaded[learnSceneCount] = { false }; // Track loaded state of each learn subscene

// Menu Scenes
static void menuInit() {
    // Initialise buttons with positions and sizes, can be made scalable with screen size later
    bPlay = { 200, 140, 220, 44 };
    bOptions = { 200, 200, 220, 44 };
    bQuit = { 200, 260, 220, 44 };
}
static void menuUnload() { /* unload menu resources */ }
static void menuUpdate()
{

    // Scene Changers
	if (button(bPlay, "Learn")) sceneManagerChangeLearnScene(LEARN_MENU); // If learn is pressed execute the learn menu scenes which executes learn menu init which sets currentScene to learn and currentLearnScene to learn menu
    if (button(bOptions, "Options")) sceneManagerChangeScene(SCENE_OPTIONS);
    if (button(bQuit, "Quit")) sceneManagerChangeScene(SCENE_EXIT);
}
static void menuDraw()
{
    ClearBackground(RAYWHITE);
    DrawText("DrawCAL - Main Menu", 180, 60, 36, MAROON);
    DrawFPS(10, 10);
}

// Learn Menu Scenes
static void learnMenuInit() {
    // Initialise buttons with positions and sizes, can be made scalable with screen size later
    bFreeDraw = { 200, 140, 220, 44 };
    bGuided = { 200, 200, 220, 44 };
    bTutorial = { 200, 260, 220, 44 };
    bBack = { 200, 320, 220, 44 };
}
static void learnMenuUnload() { /* unload learn menu resources */ }
static void learnMenuUpdate() {
    if (currentLearnScene != LEARN_MENU)
        return;
    // Scene Changers (use learn-specific helper to request learn subscenes)
    if (button(bFreeDraw, "Free Draw")) { TraceLog(LOG_INFO, "Learn menu: Free Draw pressed"); sceneManagerChangeLearnScene(LEARN_FREEDRAW); }
    if (button(bGuided, "Guided")) sceneManagerChangeLearnScene(LEARN_GUIDED);
    if (button(bTutorial, "Tutorial")) sceneManagerChangeLearnScene(LEARN_TUTORIAL);
    if (button(bBack, "Back")) sceneManagerChangeScene(SCENE_MENU);
}
static void learnMenuDraw()
{
    ClearBackground(RAYWHITE);
    DrawText("Learn - Choose a mode", 180, 60, 36, MAROON);
    DrawFPS(10, 10);
}

// Scene Registration and Switching
void sceneManagerInit()
{
    // Register top-level scenes
    scenes[(int)SCENE_MENU] = { menuInit,  menuUpdate,  menuDraw,  menuUnload };
    // scenes[(int)SCENE_OPTIONS] = { optionInit, optionUpdate, optionDraw, optionUnload };
    scenes[(int)SCENE_LEARN] = { learnMenuInit, learnMenuUpdate, learnMenuDraw, learnMenuUnload };
    scenes[(int)SCENE_EXIT] = { nullptr, nullptr, nullptr, nullptr };

    // Register learn subscenes separately
    learnScenes[(int)LEARN_MENU] = { learnMenuInit, learnMenuUpdate, learnMenuDraw, learnMenuUnload };
    learnScenes[(int)LEARN_FREEDRAW] = { freeDrawInit, freeDrawUpdate, freeDrawDraw, freeDrawUnload };
    // learnScenes[(int)LEARN_GUIDED] = { guidedInit, guidedUpdate, guidedDraw, guidedUnload };
    // learnScenes[(int)LEARN_TUTORIAL] = { tutorialInit, tutorialUpdate, tutorialDraw, tutorialUnload };
    learnScenes[(int)LEARN_EXIT] = { nullptr, nullptr, nullptr, nullptr };

    // initialize first scene immediately
    currentScene = SCENE_MENU;
    if (scenes[(int)currentScene].Init) scenes[(int)currentScene].Init();
    started = true;
}

// change pending scene for top level (safe to call from inside a scene)
void sceneManagerChangeScene(sceneId id)
{
//    if (id == SCENE_NONE || id == pendingScene || id == currentScene) return;
    pendingScene = id;
}

// change pending scene for learn subscene (safe to call from inside a learn subscene, will also request switch to LEARN top-level if not already there)

void sceneManagerChangeLearnScene(learnSceneId id)
{
//    if (id == LEARN_NONE) return;
    TraceLog(LOG_INFO, "sceneManagerChangeLearnScene -> %d", id);
    // ignore if already pending or already active
    //if (pendingLearnScene == id) return;
    //if (currentScene == SCENE_LEARN && currentLearnScene == id) return;
    pendingLearnScene = id;
    pendingScene = SCENE_LEARN;
}
sceneId sceneManagerGetCurrent() { return currentScene; }

// Run update/draw for current scene, perform pending change at frame boundary
void sceneManagerUpdate()
{
    if (!started) sceneManagerInit();

    // Apply pending top-level change
    if (pendingScene != SCENE_NONE)
    {
        TraceLog(LOG_INFO, "Applying pendingScene %d pendingLearn %d", pendingScene, pendingLearnScene);
        int pendingIdx = (int)pendingScene;
        if (pendingIdx < 0 || pendingIdx >= sceneCount) { pendingScene = SCENE_NONE; }
        else
        {
            // Leaving currentScene: unload appropriately
            if (currentScene == SCENE_LEARN)
            {
                // unload active learn subscene if any
                int ls = (int)currentLearnScene;
                if (ls >= 0 && ls < learnSceneCount && learnLoaded[ls])
                {
                    if (learnScenes[ls].Unload) learnScenes[ls].Unload();
                    learnLoaded[ls] = false;
                }
                // unload learn menu UI
                int menuIdx = (int)SCENE_LEARN;
                if (sceneLoaded[menuIdx] && scenes[menuIdx].Unload) { scenes[menuIdx].Unload(); sceneLoaded[menuIdx] = false; }
                currentLearnScene = LEARN_NONE;
            }
            else
            {
                int ci = (int)currentScene;
                if (ci >= 0 && ci < sceneCount && sceneLoaded[ci])
                {
                    if (scenes[ci].Unload) scenes[ci].Unload();
                    sceneLoaded[ci] = false;
                }
            }

            // Now enter pendingScene
            if (pendingScene == SCENE_LEARN)
            {
                // init learn menu UI
                int menuIdx = (int)SCENE_LEARN;
                if (!sceneLoaded[menuIdx] && scenes[menuIdx].Init) { scenes[menuIdx].Init(); sceneLoaded[menuIdx] = true; }

                // choose learn subscene (default to LEARN_MENU if none requested)
                learnSceneId pick = (pendingLearnScene != LEARN_NONE) ? pendingLearnScene : LEARN_MENU;
                int li = (int)pick;
                if (li >= 0 && li < learnSceneCount && learnScenes[li].Init)
                {
                    learnScenes[li].Init();
                    learnLoaded[li] = true;
                    currentLearnScene = pick;
                }
                pendingLearnScene = LEARN_NONE;
                currentScene = SCENE_LEARN;
            }
            else
            {
                int idx = (int)pendingScene;
                if (idx >= 0 && idx < sceneCount && scenes[idx].Init) { scenes[idx].Init(); sceneLoaded[idx] = true; }
                currentScene = pendingScene;
            }
        }

        if (pendingScene == SCENE_EXIT) { WindowShouldClose(); pendingScene = SCENE_NONE; return; }
        pendingScene = SCENE_NONE;
    }

    // Dispatch Update
    if (currentScene == SCENE_LEARN)
    {
        int menuIdx = (int)SCENE_LEARN;
        if (menuIdx >= 0 && menuIdx < sceneCount && scenes[menuIdx].Update) scenes[menuIdx].Update();

        int ls = (int)currentLearnScene;
        if (ls >= 0 && ls < learnSceneCount && learnScenes[ls].Update) learnScenes[ls].Update();
    }
    else
    {
        int ci = (int)currentScene;
        if (ci >= 0 && ci < sceneCount && scenes[ci].Update) scenes[ci].Update();
    }
}

void sceneManagerDraw()
{
    if (currentScene == SCENE_LEARN)
    {
        if (currentLearnScene == LEARN_MENU)
        {
            // Draw only the menu
            scenes[(int)SCENE_LEARN].Draw();
        }
        else
        {
            // Draw only the selected learn mode
            int ls = (int)currentLearnScene;

            if (ls >= 0 &&
                ls < learnSceneCount &&
                learnScenes[ls].Draw)
            {
                learnScenes[ls].Draw();
            }
        }
    }
    else
    {
        int ci = (int)currentScene;

        if (ci >= 0 &&
            ci < sceneCount &&
            scenes[ci].Draw)
        {
            scenes[ci].Draw();
        }
    }
}