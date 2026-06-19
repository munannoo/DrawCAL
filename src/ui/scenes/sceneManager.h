#ifndef SCENE_MANAGER
#define SCENE_MANAGER

#include "raylib.h"
#include "ui/widgets/buttons.h" // has button function for rendering buttons


// modularised functions so that everything will be organised
#include "mainMenuScenes.h"
#include "learnMenuScenes.h"
#include "mainMenuOptions.h"
#include "features/learning/FreeDrawMode.h"

#include "rendering/resolution.h" // For dynamic button size management


// Scene identifiers, -1 for none, use enum class for function overloading and to avoid enum confusions
// The count at the end is placed there for ease of count variables and dynamicability
//
// if adding anything else, add before count
enum class sceneId { SCENE_NONE = -1, SCENE_MENU = 0, SCENE_LEARN, SCENE_OPTIONS, SCENE_COUNT };
enum class learnSceneId { LEARN_NONE = -1, LEARN_MENU = 0, LEARN_FREEDRAW, LEARN_GUIDED, LEARN_TUTORIAL, LEARN_COUNT };
enum class optionSceneId { OPTIONS_NONE = -1, OPTIONS_MENU = 0, OPTIONS_GRAPHICS, OPTIONS_CONTROLS, OPTIONS_INTERFACE, OPTIONS_COUNT };
enum class graphicsSceneId { GRAPHICS_NONE = -1, GRAPHICS_MENU = 0, GRAPHICS_RESOLUTION, GRAPHICS_FULLSCREEN, GRAPHICS_VSYNC, GRAPHICS_COUNT };
enum class controlsSceneId { CONTROLS_NONE = -1, CONTROLS_MENU = 0, CONTROLS_REMAPPING, CONTROLS_SENSITIVITY, CONTROL_GUIDELINES, CONTROLS_COUNT };
enum class interfaceSceneId { INTERFACE_NONE = -1, INTERFACE_MENU = 0, INTERFACE_THEME, INTERFACE_FONTS, INTERFACE_COUNT };



// have function pointers inside structures so that all 4 functions may be executed via the struct
struct sceneFunctions {
    void (*Init)();
    void (*Update)();
    void (*Draw)();
    void (*Unload)();
};


extern Rectangle btnGraphics, btnControls, btnInterface;
extern Rectangle btnPlay, btnEditor, btnOptions, btnExit;
extern Rectangle btnFreeDraw, btnGuided, btnTutorial, btnBack;

extern float btnWidth;   
extern float btnHeight;  
extern float btnLeft;    
extern float btnTop;     
extern float btnGap;     // initialized in sceneManager.cpp

extern sceneId currentScene, pendingScene;
extern learnSceneId currentLearnScene, pendingLearnScene;
extern optionSceneId currentOptionScene, pendingOptionScene;
extern graphicsSceneId currentGraphicsScene, pendingGraphicsScene;
extern controlsSceneId currentControlsScene, pendingControlsScene;
extern interfaceSceneId currentInterfaceScene, pendingInterfaceScene;

extern bool sceneInitialized;
  

// Request a scene change (deferred to the frame boundary)
// Maybe can template this but idk how

void sceneManagerChangeScene(sceneId);
void sceneManagerChangeScene(learnSceneId);
void sceneManagerChangeScene(optionSceneId);


void sceneManagerInit(); // Initialize scene manager, register scenes
void sceneManagerUpdate(); // Call every frame: runs Update for active scene and performs switchover
void sceneManagerDraw(); // Call every frame: runs Draw for active scene

#endif // !SCENE_MANAGER

