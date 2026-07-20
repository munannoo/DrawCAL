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

enum GuidedButtonIcon
{
    BUTTON_ICON_CUBE,
    BUTTON_ICON_SPHERE,
    BUTTON_ICON_CYLINDER,
    BUTTON_ICON_IMPORT
};

static void OpenWorkspaceWithObject(int objectType)
{
    const Vector3 spawnPosition = { 0.0f, 0.0f, 0.0f };

    clearScene();

    shape* newObject = nullptr;

    switch (objectType)
    {
    case 1: objects.push_back(std::make_unique<cube>(spawnPosition));     newObject = objects.back().get(); break;
    case 2: objects.push_back(std::make_unique<sphere>(spawnPosition));   newObject = objects.back().get(); break;
    case 3: objects.push_back(std::make_unique<cylinder>(spawnPosition)); newObject = objects.back().get(); break;
    default:
        TraceLog(LOG_WARNING, "Guided mode: object type %d not yet implemented", objectType);
        return;
    }

    lights.push_back(std::make_unique<Light>(
        Vector3{ 6.0f, 20.0f, 10.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, WHITE, R3D_LIGHT_SPOT));

    if (newObject != nullptr)
        selectObjects(newObject, false);

    SetGuidedWorkspace(true);
    sceneManagerChangeScene(learnSceneId::LEARN_FREEDRAW);
}


static void DrawCubeIcon(Rectangle r)
{
    Color c = Fade(WHITE, 0.22f);

    float size = r.width * 0.32f;

    Vector2 f1 = { r.x + r.width * 0.33f, r.y + r.height * 0.26f };
    Vector2 f2 = { f1.x + size, f1.y };
    Vector2 f3 = { f2.x, f2.y + size };
    Vector2 f4 = { f1.x, f1.y + size };

    Vector2 off = { size * 0.25f, -size * 0.25f };

    Vector2 b1 = Vector2Add(f1, off);
    Vector2 b2 = Vector2Add(f2, off);
    Vector2 b3 = Vector2Add(f3, off);
    Vector2 b4 = Vector2Add(f4, off);

    const float t = 2.0f;

    DrawLineEx(f1, f2, t, c);
    DrawLineEx(f2, f3, t, c);
    DrawLineEx(f3, f4, t, c);
    DrawLineEx(f4, f1, t, c);

    DrawLineEx(b1, b2, t, c);
    DrawLineEx(b2, b3, t, c);
    DrawLineEx(b3, b4, t, c);
    DrawLineEx(b4, b1, t, c);

    DrawLineEx(f1, b1, t, c);
    DrawLineEx(f2, b2, t, c);
    DrawLineEx(f3, b3, t, c);
    DrawLineEx(f4, b4, t, c);
}

static void DrawSphereIcon(Rectangle r)
{
    Color c = Fade(WHITE, 0.22f);

    Vector2 center =
    {
        r.x + r.width / 2,
        r.y + r.height * 0.40f
    };

    float radius = r.width * 0.19f;

    DrawCircleLinesV(center, radius, c);

    DrawEllipseLines(
        (int)center.x,
        (int)center.y,
        radius * 0.45f,
        radius,
        c);

    DrawEllipseLines(
        (int)center.x,
        (int)center.y,
        radius,
        radius * 0.35f,
        c);
}

static void DrawCylinderIcon(Rectangle r)
{
    Color c = Fade(WHITE, 0.22f);

    float w = r.width * 0.34f;
    float h = r.height * 0.32f;

    float x = r.x + r.width / 2;
    float y = r.y + r.height * 0.22f;

    DrawEllipseLines((int)x, (int)y, w / 2, 8, c);

    DrawLineEx({ x - w / 2, y }, { x - w / 2, y + h }, 2, c);
    DrawLineEx({ x + w / 2, y }, { x + w / 2, y + h }, 2, c);

    DrawEllipseLines((int)x, (int)(y + h), w / 2, 8, c);
}

static void DrawImportIcon(Rectangle r)
{
    Color c = Fade(WHITE, 0.22f);

    float cx = r.x + r.width / 2;
    float cy = r.y + r.height * 0.36f;

    DrawLineEx({ cx, cy + 18 }, { cx, cy - 10 }, 3, c);
    DrawLineEx({ cx, cy - 10 }, { cx - 10, cy }, 3, c);
    DrawLineEx({ cx, cy - 10 }, { cx + 10, cy }, 3, c);

    DrawLineEx({ cx - 20, cy + 22 }, { cx + 20, cy + 22 }, 3, c);
}
//---------------------------------------------------------
// Draw a custom rounded button
//---------------------------------------------------------
static void DrawRoundedButton(Rectangle bounds,
                              Color color,
                              const char* text,
                              GuidedButtonIcon icon)
{
    bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);

    Rectangle drawBounds = bounds;

    if (hovered)
    {
        drawBounds.x -= 2;
        drawBounds.y -= 2;
        drawBounds.width += 4;
        drawBounds.height += 4;
    }

    Color drawColor = hovered
                        ? Fade(color, 0.85f)
                        : color;

    DrawRectangleRounded(drawBounds, 0.20f, 8, drawColor);

    DrawRectangleRoundedLines(
        drawBounds,
        0.20f,
        8,
        GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

    switch (icon)
{
    case BUTTON_ICON_CUBE:
        DrawCubeIcon(drawBounds);
        break;

    case BUTTON_ICON_SPHERE:
        DrawSphereIcon(drawBounds);
        break;

    case BUTTON_ICON_CYLINDER:
        DrawCylinderIcon(drawBounds);
        break;

    case BUTTON_ICON_IMPORT:
        DrawImportIcon(drawBounds);
        break;
}

    const int fontSize = 22;

    Vector2 textSize = MeasureThemeText(text, (float)fontSize);

    DrawThemeText(
        text,
        drawBounds.x + (drawBounds.width - textSize.x) / 2,
        drawBounds.y + drawBounds.height - 38,
        (float)fontSize,
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
        loadScene();
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

    DrawRoundedButton(cubeBtn,
                  Color{52,152,219,255},
                  "Cube",
                  BUTTON_ICON_CUBE);

    DrawRoundedButton(sphereBtn,
                  Color{46,204,113,255},
                  "Sphere",
                  BUTTON_ICON_SPHERE);

    DrawRoundedButton(cylinderBtn,
                  Color{230,126,34,255},
                  "Cylinder",
                  BUTTON_ICON_CYLINDER);

    DrawRoundedButton(importBtn,
                  Color{155,89,182,255},
                  "Import",
                  BUTTON_ICON_IMPORT);
}

void GuidedModeUnload()
{
    // Nothing to unload
}
