#include "layout.h"

#include "features/camera/CameraController.h"
#include "objects/object.h"
#include "raygui.h"
#include "ui/scenes/sceneManager.h"

float editorToolbarHeight = 82.0f;

Rectangle GetViewSelectorBounds()
{
    return { (float)GetScreenWidth() - 210.0f, 36.0f, 150.0f, 34.0f };
}

Rectangle GetEditorDockBounds()
{
    float width = GetScreenWidth() * 0.30f;
    if (width < 370.0f) width = 370.0f;
    if (width > 430.0f) width = 430.0f;

    return { (float)GetScreenWidth() - width, editorToolbarHeight, width,
             (float)GetScreenHeight() - editorToolbarHeight };
}

Rectangle GetWorkspacePanelBounds(bool propertiesVisible)
{
    Rectangle dock = GetEditorDockBounds();
    if (!propertiesVisible) return dock;

    dock.height = dock.height * 0.42f - 3.0f;
    return dock;
}

Rectangle GetPropertiesPanelBounds(bool workspaceVisible)
{
    Rectangle dock = GetEditorDockBounds();
    if (!workspaceVisible) return dock;

    float workspaceHeight = dock.height * 0.42f - 3.0f;
    dock.y += workspaceHeight + 6.0f;
    dock.height -= workspaceHeight + 6.0f;
    return dock;
}

bool IsPointerOverEditorLayout(bool workspaceVisible, bool propertiesVisible,
                               bool guidedWorkspace, bool viewDropdownOpen)
{
    Vector2 mouse = GetMousePosition();
    Rectangle toolbar = { 0.0f, 0.0f, (float)GetScreenWidth(), editorToolbarHeight };

    if (CheckCollisionPointRec(mouse, toolbar)) return true;

    if (viewDropdownOpen)
    {
        Rectangle menu = GetViewSelectorBounds();
        float spacing = (float)GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_SPACING);
        menu.height = 6.0f * (menu.height + spacing);
        if (CheckCollisionPointRec(mouse, menu)) return true;
    }

    if (guidedWorkspace || workspaceVisible || propertiesVisible)
        return CheckCollisionPointRec(mouse, GetEditorDockBounds());

    return false;
}

bool DrawEditorPanel(Rectangle bounds, const char* title)
{
    Color background = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
    Color border = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
    Color header = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));

    DrawRectangleRec(bounds, Fade(background, 0.97f));
    DrawRectangleLinesEx(bounds, 1.0f, border);
    DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, 34, header);
    GuiLabel({ bounds.x + 10.0f, bounds.y + 2.0f, bounds.width - 48.0f, 30.0f }, title);

    Rectangle close = { bounds.x + bounds.width - 30.0f, bounds.y + 4.0f,
                        26.0f, 26.0f };
    return GuiButton(close, GuiIconText(ICON_CROSS_SMALL, NULL));
}

bool DrawToolbarButton(float& x, const char* text, float width)
{
    bool pressed = GuiButton({ x, 36.0f, width, 34.0f }, text);
    x += width + 6.0f;
    return pressed;
}

void DrawToolbarDivider(float& x)
{
    x += 7.0f;
    Color border = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
    DrawLine((int)x, 9, (int)x, 72, Fade(border, 0.7f));
    x += 13.0f;
}

void DrawToolbarLabel(float x, float width, const char* text)
{
    GuiLabel({ x, 7.0f, width, 22.0f }, text);
}

bool InsertToolbarObject(int type, bool isLight, Camera3D& camera)
{
    int oldCount = getObjectCount(type);

    if (type == 1) cube(camera.target, GRAY);
    else if (type == 2 && isLight) lightSphere(camera.target, WHITE);
    else if (type == 2) sphere(camera.target, GRAY);
    else if (type == 3) cylinder(camera.target, GRAY);

    int newCount = getObjectCount(type);
    if (newCount == oldCount) return false;

    selectObject(type, newCount - 1);
    return true;
}

void SetCameraView(Camera3D& camera, Vector3 position, Vector3 up, int projection)
{
    camera.position = position;
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = up;
    camera.projection = projection;
    camera.fovy = 45.0f;
}

void UpdateCameraView(Camera3D& camera, viewIndex view, bool& cameraLocked)
{
    cameraLocked = true;

    switch (view)
    {
        case VIEW_FREE:
            InitCamera(camera);
            cameraLocked = false;
            break;
        case VIEW_FRONT:
            SetCameraView(camera, { 0.0f, 0.0f, 10.0f },
                          { 0.0f, 1.0f, 0.0f }, CAMERA_PERSPECTIVE);
            break;
        case VIEW_TOP:
            SetCameraView(camera, { 0.0f, 10.0f, 0.0f },
                          { 0.0f, 0.0f, -1.0f }, CAMERA_ORTHOGRAPHIC);
            break;
        case VIEW_LEFT:
            SetCameraView(camera, { -10.0f, 0.0f, 0.0f },
                          { 0.0f, 1.0f, 0.0f }, CAMERA_PERSPECTIVE);
            break;
        case VIEW_RIGHT:
            SetCameraView(camera, { 10.0f, 0.0f, 0.0f },
                          { 0.0f, 1.0f, 0.0f }, CAMERA_PERSPECTIVE);
            break;
        default:
            break;
    }
}

void DrawViewSelector(Camera3D& camera, viewIndex& currentView,
                      viewIndex& lastView, bool& dropdownOpen, bool& cameraLocked)
{
    int selected = (int)currentView;
    if (GuiDropdownBox(GetViewSelectorBounds(), "Free;Front;Top;Left;Right",
                       &selected, dropdownOpen))
        dropdownOpen = !dropdownOpen;

    currentView = (viewIndex)selected;
    if (currentView == lastView) return;

    UpdateCameraView(camera, currentView, cameraLocked);
    lastView = currentView;
}

bool DrawEditorToolbar(bool& workspaceVisible, bool& propertiesVisible,
                       Camera3D& camera, viewIndex& currentView,
                       viewIndex& lastView, bool& dropdownOpen,
                       bool& cameraLocked, bool guidedWorkspace)
{
    Color background = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
    Color border = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
    DrawRectangle(0, 0, GetScreenWidth(), (int)editorToolbarHeight,
                  Fade(background, 0.98f));
    DrawLine(0, (int)editorToolbarHeight - 1, GetScreenWidth(),
             (int)editorToolbarHeight - 1, border);

    bool changed = false;

    if (!guidedWorkspace)
    {
        float x = 12.0f;
        DrawToolbarLabel(x, 134.0f, "FILE");
        if (DrawToolbarButton(x, "Save", 64.0f)) save();
        if (DrawToolbarButton(x, "Load", 64.0f))
        {
            load();
            deselectAllObjects();
            changed = true;
        }

        DrawToolbarDivider(x);
        DrawToolbarLabel(x, 324.0f, "CREATE");
        if (DrawToolbarButton(x, "+ Cube", 70.0f))
            changed = InsertToolbarObject(1, false, camera);
        if (DrawToolbarButton(x, "+ Sphere", 78.0f))
            changed = InsertToolbarObject(2, false, camera);
        if (DrawToolbarButton(x, "+ Cylinder", 86.0f))
            changed = InsertToolbarObject(3, false, camera);
        if (DrawToolbarButton(x, "+ Light", 72.0f))
            changed = InsertToolbarObject(2, true, camera);

        DrawToolbarDivider(x);
        DrawToolbarLabel(x, 72.0f, "EDIT");
        if (DrawToolbarButton(x, "Delete", 72.0f))
        {
            deleteobj();
            changed = true;
        }

        DrawToolbarDivider(x);
        DrawToolbarLabel(x, 210.0f, "PANELS");
        GuiToggle({ x, 36.0f, 104.0f, 34.0f }, "Workspace", &workspaceVisible);
        x += 110.0f;
        GuiToggle({ x, 36.0f, 100.0f, 34.0f }, "Properties", &propertiesVisible);
    }

    Rectangle view = GetViewSelectorBounds();
    DrawToolbarLabel(view.x, view.width, "VIEW");
    DrawViewSelector(camera, currentView, lastView, dropdownOpen, cameraLocked);

    Rectangle settings = { (float)GetScreenWidth() - 46.0f, 36.0f, 34.0f, 34.0f };
    if (GuiButton(settings, ""))
        sceneManagerChangeScene(sceneId::SCENE_OPTIONS);
    GuiDrawIcon(ICON_GEAR_BIG, (int)settings.x, (int)settings.y, 2,
                GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

    if (changed)
    {
        workspaceVisible = true;
        propertiesVisible = true;
    }
    return changed;
}
