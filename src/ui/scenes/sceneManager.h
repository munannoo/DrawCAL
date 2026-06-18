#ifndef SCENE_MANAGER
#define SCENE_MANAGER

#include "raylib.h"
#include "ui/widgets/buttons.h" // has button function for rendering buttons


// modularised functions so that everything will be organised
#include "mainMenuScenes.h"
#include "learnMenuScenes.h"
#include "features/learning/FreeDrawMode.h"

#include "rendering/resolution.h" // For dynamic button size management


// Scene identifiers, -1 for none, use enum class for function overloading and to avoid enum confusions
enum class sceneId { SCENE_NONE = -1, SCENE_MENU = 0, SCENE_LEARN, SCENE_OPTIONS, SCENE_EXIT };
enum class learnSceneId { LEARN_NONE = -1, LEARN_MENU = 0, LEARN_FREEDRAW, LEARN_GUIDED, LEARN_TUTORIAL, LEARN_EXIT };

// have function pointers inside structures so that all 4 functions may be executed with just the struct
struct sceneFunctions {
    void (*Init)();
    void (*Update)();
    void (*Draw)();
    void (*Unload)();
};

const int sceneCount = 4; // Update this if you add more scenes
const int learnSceneCount = 5; // Update this if you add more learn subscenes

extern Rectangle btnPlay, btnEditor, btnOptions, btnExit;
extern Rectangle btnFreeDraw, btnGuided, btnTutorial, btnBack;

extern float btnWidth;   // 16% of width (initialized in sceneManager.cpp)
extern float btnHeight;  // 7% of height (initialized in sceneManager.cpp)
extern float btnLeft;    // 8% margin from left (initialized in sceneManager.cpp)
extern float btnTop;     // starting top (initialized in sceneManager.cpp)
extern float btnGap;     // initialized in sceneManager.cpp

extern sceneId currentScene;
extern sceneId pendingScene;
extern learnSceneId currentLearnScene;
extern learnSceneId pendingLearnScene;

extern bool sceneInitialized;
extern bool learnSceneInitialized;

// Initialize scene manager, register scenes
void sceneManagerInit();
// Request a scene change (deferred to the frame boundary)
void sceneManagerChangeScene(sceneId);
void sceneManagerChangeScene(learnSceneId);
// Call every frame: runs Update for active scene and performs switchover
void sceneManagerUpdate();
// Call every frame: runs Draw for active scene
void sceneManagerDraw(); 
sceneId sceneManagerGetCurrent();

#endif // !SCENE_MANAGER

