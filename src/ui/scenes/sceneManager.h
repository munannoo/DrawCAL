#ifndef SCENE_MANAGER
#define SCENE_MANAGER

#include "raylib.h"
#include "ui/widgets/buttons.h"
#include "features/learning/FreeDrawMode.h"

// Scene identifiers
enum sceneId { SCENE_NONE = -1, SCENE_MENU = 0, SCENE_LEARN, SCENE_OPTIONS, SCENE_EXIT };
enum learnSceneId { LEARN_NONE = -1, LEARN_MENU = 0, LEARN_FREEDRAW, LEARN_GUIDED, LEARN_TUTORIAL, LEARN_EXIT };

// have functions inside structures so that all 4 functions may be executed with just the struct
struct scene {
    void (*Init)();
    void (*Update)();
    void (*Draw)();
    void (*Unload)();
};

const int sceneCount = 4; // Update this if you add more scenes
const int learnSceneCount = 5; // Update this if you add more learn subscenes
// Initialize scene manager, register scenes
void sceneManagerInit();
// Request a scene change (deferred to the frame boundary)
void sceneManagerChangeScene(sceneId);
void sceneManagerChangeLearnScene(learnSceneId);
// Call every frame: runs Update for active scene and performs switchover
void sceneManagerUpdate();
// Call every frame: runs Draw for active scene
void sceneManagerDraw(); 
sceneId sceneManagerGetCurrent();

#endif // !SCENE_MANAGER

