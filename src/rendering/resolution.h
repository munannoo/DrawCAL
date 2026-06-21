#ifndef resolution_h
#define resolution_h

#include <string> // Include string for string name in resolutionClass
#include "raylib.h"
#include "raygui.h"
#include "ui/scenes/sceneManager.h" // For dynamic button size management, as the button sizes are determined by the resolution and need to be updated when resolution changes

extern int currentResIndex;
extern int lastResIndex;

typedef struct resolutions {
    int width;
    int height;
    std::string name;
} resolutionClass;

// Add enum for readibility in value assigning
enum class resolutionIndex {
    RES_720p,
    RES_900p,
    RES_1080p,
    RES_1440p,
    RES_1600p
};

extern const resolutionClass resolutions[5];

void changeButtonResolution();

#endif // resolution_h