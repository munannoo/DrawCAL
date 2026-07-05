#include "raylib.h"
#include "raygui.h"
#include "features/learning/GuidedMode.h"
#include "ui/scenes/sceneManager.h"

static Rectangle cubeBtn;
static Rectangle sphereBtn;
static Rectangle cylinderBtn;
static Rectangle importBtn;
static Rectangle backBtn;

//---------------------------------------------------------
// Draw a custom rounded button
//---------------------------------------------------------
void DrawRoundedButton(Rectangle bounds, Color color, const char* text)
{
    bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);

    Color drawColor = hovered ? Fade(color, 0.8f) : color;

    DrawRectangleRounded(bounds, 0.20f, 8, drawColor);

    // Optional border
    DrawRectangleRoundedLines(bounds, 0.20f, 8, BLACK);

    int fontSize = 22;

    DrawText(
        text,
        bounds.x + (bounds.width - MeasureText(text, fontSize)) / 2,
        bounds.y + bounds.height - 38,
        fontSize,
        WHITE
    );
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
        // TODO: Create Cube
    }

    if (CheckCollisionPointRec(GetMousePosition(), sphereBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        // TODO: Create Sphere
    }

    if (CheckCollisionPointRec(GetMousePosition(), cylinderBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        // TODO: Create Cylinder
    }

    if (CheckCollisionPointRec(GetMousePosition(), importBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        // TODO: Import Model
    }
    if (CheckCollisionPointRec(GetMousePosition(), backBtn) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        pendingLearnScene = learnSceneId::LEARN_MENU;
    }
}

void GuidedModeDraw()
{
   ClearBackground(RAYWHITE);
   DrawRoundedButton(backBtn, GRAY, "< Back");

    const char* title = "Guided Mode";

    DrawText(
        title,
        (GetScreenWidth() - MeasureText(title, 40)) / 2,
        80,
        40,
        DARKGRAY
    );

    DrawRoundedButton(cubeBtn, Color{52, 152, 219, 255}, "Cube");
    DrawRoundedButton(sphereBtn, Color{46, 204, 113, 255}, "Sphere");
    DrawRoundedButton(cylinderBtn, Color{230, 126, 34, 255}, "Cylinder");
    DrawRoundedButton(importBtn, Color{155, 89, 182, 255}, "Import");
}

void GuidedModeUnload()
{
    // Nothing to unload
}
