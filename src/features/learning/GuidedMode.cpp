#include "raylib.h"
#include "raygui.h"
#include "features/learning/GuidedMode.h"
#include "objects/object.h"
#include "ui/scenes/sceneManager.h"
#include "ui/themes/themes.h"

static Rectangle cubeBtn;
static Rectangle sphereBtn;
static Rectangle cylinderBtn;
static Rectangle importBtn;
static Rectangle backBtn;

static void OpenWorkspaceWithObject(int objectType)
{
    const Vector3 spawnPosition = { 0.0f, 0.0f, 0.0f };
    const int previousCount = getObjectCount(objectType);

    switch (objectType)
    {
        case 1: cube(spawnPosition); break;
        case 2: sphere(spawnPosition); break;
        case 3: cylinder(spawnPosition); break;
        default: return;
    }

    // Highlight the object that was just created when the workspace opens.
    const int newCount = getObjectCount(objectType);
    if (newCount > previousCount)
        selectObject(objectType, newCount - 1);
    SetGuidedWorkspace(true);
    sceneManagerChangeScene(learnSceneId::LEARN_FREEDRAW);
}

//---------------------------------------------------------
// Draw a custom rounded button
//---------------------------------------------------------
void DrawRoundedButton(Rectangle bounds, Color color, const char* text)
{
    bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);

    Color drawColor = hovered ? Fade(color, 0.8f) : color;

    DrawRectangleRounded(bounds, 0.20f, 8, drawColor);

    // Optional border
    DrawRectangleRoundedLines(bounds, 0.20f, 8,
        GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

    int fontSize = 22;

    Vector2 textSize = MeasureThemeText(text, (float)fontSize);
    DrawThemeText(text, bounds.x + (bounds.width - textSize.x) / 2.0f,
                  bounds.y + bounds.height - 38.0f, (float)fontSize,
                  GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
}

void GuidedModeInit()
{
    const float buttonSize = 170.0f;
    const float spacing = 30.0f;

    float totalWidth = buttonSize * 2 + spacing;
    float totalHeight = buttonSize * 2 + spacing;

    float startX = (GetScreenWidth() - totalWidth) / 2;
    float startY = (GetScreenHeight() - totalHeight) / 2;

    cubeBtn = { startX,
                startY,
                buttonSize,
                buttonSize };

    sphereBtn = { startX + buttonSize + spacing,
                  startY,
                  buttonSize,
                  buttonSize };

    cylinderBtn = { startX,
                    startY + buttonSize + spacing,
                    buttonSize,
                    buttonSize };

    importBtn = { startX + buttonSize + spacing,
                  startY + buttonSize + spacing,
                  buttonSize,
                  buttonSize };
    backBtn = {
        20,
        20,
        110,
        45
    };
}

void GuidedModeUpdate()
{
    if (CheckCollisionPointRec(GetMousePosition(), cubeBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        OpenWorkspaceWithObject(1);
    }

    if (CheckCollisionPointRec(GetMousePosition(), sphereBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        OpenWorkspaceWithObject(2);
    }

    if (CheckCollisionPointRec(GetMousePosition(), cylinderBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        OpenWorkspaceWithObject(3);
    }

    if (CheckCollisionPointRec(GetMousePosition(), importBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        load();
        SetGuidedWorkspace(true);
        sceneManagerChangeScene(learnSceneId::LEARN_FREEDRAW);
    }
    if (CheckCollisionPointRec(GetMousePosition(), backBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        pendingLearnScene = learnSceneId::LEARN_MENU;
    }
}

void GuidedModeDraw()
{
   ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
   if (GuiButton(backBtn, "< Back")) pendingLearnScene = learnSceneId::LEARN_MENU;

    const char* title = "Guided Mode";

    Vector2 titleSize = MeasureThemeText(title, 40.0f);
    DrawThemeText(title, (GetScreenWidth() - titleSize.x) / 2.0f, 80.0f, 40.0f,
                  GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

    DrawRoundedButton(cubeBtn, Color{52, 152, 219, 255}, "Cube");
    DrawRoundedButton(sphereBtn, Color{46, 204, 113, 255}, "Sphere");
    DrawRoundedButton(cylinderBtn, Color{230, 126, 34, 255}, "Cylinder");
    DrawRoundedButton(importBtn, Color{155, 89, 182, 255}, "Import");
}

void GuidedModeUnload()
{
    // Nothing to unload
}
