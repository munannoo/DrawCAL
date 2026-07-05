#include "raylib.h"
#include "raygui.h"
#include "features/learning/GuidedMode.h"

static Rectangle cubeBtn;
static Rectangle sphereBtn;
static Rectangle cylinderBtn;
static Rectangle importBtn;

void GuidedModeInit()
{
    cubeBtn     = {20, 20, 120, 35};
    sphereBtn   = {20, 65, 120, 35};
    cylinderBtn = {20,110,120,35};
    importBtn   = {20,155,120,35};
}

void GuidedModeUpdate()
{
    if (GuiButton(cubeBtn, "Cube"))
    {
        // Create cube
    }

    if (GuiButton(sphereBtn, "Sphere"))
    {
        // Create sphere
    }

    if (GuiButton(cylinderBtn, "Cylinder"))
    {
        // Create cylinder
    }

    if (GuiButton(importBtn, "Import"))
    {
        // Import model
    }
}

void GuidedModeDraw()
{
    GuiButton(cubeBtn, "Cube");
    GuiButton(sphereBtn, "Sphere");
    GuiButton(cylinderBtn, "Cylinder");
    GuiButton(importBtn, "Import");
}

void GuidedModeUnload()
{
    // Nothing to unload
}
