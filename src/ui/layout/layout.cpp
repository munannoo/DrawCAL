#include "layout.h"

#include "features/camera/CameraController.h"
#include "objects/object.h"
#include "raygui.h"
#include "ui/scenes/sceneManager.h"

float GetEditorToolbarHeight()
{
    return 58.0f;
}

Rectangle GetEditorDockBounds()
{
    float width = GetScreenWidth() * 0.30f;

    if (width < 370.0f) width = 370.0f;
    if (width > 430.0f) width = 430.0f;

    return {
        (float)GetScreenWidth() - width,
        GetEditorToolbarHeight(),
        width,
        (float)GetScreenHeight() - GetEditorToolbarHeight()
    };
}

Rectangle GetWorkspacePanelBounds(bool propertiesVisible)
{
    Rectangle dock = GetEditorDockBounds();

    if (!propertiesVisible) return dock;

    return {
        dock.x,
        dock.y,
        dock.width,
        dock.height * 0.42f - 3.0f
    };
}

Rectangle GetPropertiesPanelBounds(bool workspaceVisible)
{
    Rectangle dock = GetEditorDockBounds();

    if (!workspaceVisible) return dock;

    float workspaceHeight = dock.height * 0.42f - 3.0f;

    return {
        dock.x,
        dock.y + workspaceHeight + 6.0f,
        dock.width,
        dock.height - workspaceHeight - 6.0f
    };
}

Rectangle GetViewSelectorBounds()
{
    float width = 150.0f;

    return {
        (float)GetScreenWidth() - width - 60.0f,
        12.0f,
        width,
        34.0f
    };
}

bool IsPointerOverEditorLayout(
    bool workspaceVisible,
    bool propertiesVisible,
    bool guidedWorkspace,
    bool viewDropdownOpen
)
{
    Vector2 mouse = GetMousePosition();

    Rectangle toolbar = {
        0.0f,
        0.0f,
        (float)GetScreenWidth(),
        GetEditorToolbarHeight()
    };

    if (CheckCollisionPointRec(mouse, toolbar)) return true;

    if (viewDropdownOpen)
    {
        Rectangle viewMenu = GetViewSelectorBounds();
        float spacing = (float)GuiGetStyle(
            DROPDOWNBOX,
            DROPDOWN_ITEMS_SPACING
        );

        viewMenu.height = 6.0f * (viewMenu.height + spacing);

        if (CheckCollisionPointRec(mouse, viewMenu)) return true;
    }

    if (guidedWorkspace || workspaceVisible || propertiesVisible)
    {
        if (CheckCollisionPointRec(mouse, GetEditorDockBounds())) return true;
    }

    return false;
}

bool DrawEditorPanel(Rectangle bounds, const char* title)
{
    Color background = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
    Color border = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
    Color header = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));

    DrawRectangleRec(bounds, Fade(background, 0.97f));
    DrawRectangleLinesEx(bounds, 1.0f, border);
    DrawRectangle(
        (int)bounds.x,
        (int)bounds.y,
        (int)bounds.width,
        34,
        header
    );

    GuiLabel(
        { bounds.x + 10.0f, bounds.y + 2.0f, bounds.width - 48.0f, 30.0f },
        title
    );

    Rectangle closeButton = {
        bounds.x + bounds.width - 30.0f,
        bounds.y + 4.0f,
        26.0f,
        26.0f
    };

    return GuiButton(
        closeButton,
        GuiIconText(ICON_CROSS_SMALL, NULL)
    );
}

bool DrawToolbarButton(float& x, const char* label, float width)
{
    Rectangle button = { x, 12.0f, width, 34.0f };
    bool pressed = GuiButton(button, label);

    x += width + 6.0f;

    return pressed;
}

void DrawToolbarDivider(float& x)
{
    x += 7.0f;

    Color border = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
    DrawLine((int)x, 14, (int)x, 44, Fade(border, 0.7f));

    x += 13.0f;
}

bool InsertToolbarObject(int objectType, bool pointLight, Camera3D& camera)
{
    int oldCount = getObjectCount(objectType);
    Vector3 position = camera.target;

    if (objectType == 1)
    {
        cube(position, GRAY);
    }
    else if (objectType == 2 && pointLight)
    {
        lightSphere(position, WHITE);
    }
    else if (objectType == 2)
    {
        sphere(position, GRAY);
    }
    else if (objectType == 3)
    {
        cylinder(position, GRAY);
    }

    int newCount = getObjectCount(objectType);

    if (newCount > oldCount)
    {
        selectObject(objectType, newCount - 1);
        return true;
    }

    return false;
}

void UpdateCameraView(
    Camera3D& camera,
    viewIndex currentView,
    bool& cameraLocked
)
{
    if (currentView == VIEW_FREE)
    {
        InitCamera(camera);
        cameraLocked = false;
    }
    else if (currentView == VIEW_FRONT)
    {
        camera.position = { 0.0f, 0.0f, 10.0f };
        camera.target = { 0.0f, 0.0f, 0.0f };
        camera.up = { 0.0f, 1.0f, 0.0f };
        camera.projection = CAMERA_PERSPECTIVE;
        camera.fovy = 45.0f;
        cameraLocked = true;
    }
    else if (currentView == VIEW_TOP)
    {
        camera.position = { 0.0f, 10.0f, 0.0f };
        camera.target = { 0.0f, 0.0f, 0.0f };
        camera.up = { 0.0f, 0.0f, -1.0f };
        camera.projection = CAMERA_ORTHOGRAPHIC;
        camera.fovy = 45.0f;
        cameraLocked = true;
    }
    else if (currentView == VIEW_LEFT)
    {
        camera.position = { -10.0f, 0.0f, 0.0f };
        camera.target = { 0.0f, 0.0f, 0.0f };
        camera.up = { 0.0f, 1.0f, 0.0f };
        camera.projection = CAMERA_PERSPECTIVE;
        camera.fovy = 45.0f;
        cameraLocked = true;
    }
    else if (currentView == VIEW_RIGHT)
    {
        camera.position = { 10.0f, 0.0f, 0.0f };
        camera.target = { 0.0f, 0.0f, 0.0f };
        camera.up = { 0.0f, 1.0f, 0.0f };
        camera.projection = CAMERA_PERSPECTIVE;
        camera.fovy = 45.0f;
        cameraLocked = true;
    }
}

void DrawViewSelector(
    Camera3D& camera,
    viewIndex& currentView,
    viewIndex& lastView,
    bool& viewDropdownOpen,
    bool& cameraLocked
)
{
    Rectangle bounds = GetViewSelectorBounds();
    int selectedView = (int)currentView;

    if (GuiDropdownBox(
        bounds,
        "Free;Front;Top;Left;Right",
        &selectedView,
        viewDropdownOpen
    ))
    {
        viewDropdownOpen = !viewDropdownOpen;
    }

    currentView = (viewIndex)selectedView;

    if (currentView != lastView)
    {
        UpdateCameraView(camera, currentView, cameraLocked);
        lastView = currentView;
    }
}

bool DrawEditorToolbar(
    bool& workspaceVisible,
    bool& propertiesVisible,
    Camera3D& camera,
    viewIndex& currentView,
    viewIndex& lastView,
    bool& viewDropdownOpen,
    bool& cameraLocked,
    bool guidedWorkspace
)
{
    Color background = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
    Color border = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));

    DrawRectangle(
        0,
        0,
        GetScreenWidth(),
        (int)GetEditorToolbarHeight(),
        Fade(background, 0.98f)
    );

    DrawLine(
        0,
        (int)GetEditorToolbarHeight() - 1,
        GetScreenWidth(),
        (int)GetEditorToolbarHeight() - 1,
        border
    );

    bool sceneChanged = false;

    if (!guidedWorkspace)
    {
        float x = 12.0f;

        if (DrawToolbarButton(x, "Save", 64.0f)) save();

        if (DrawToolbarButton(x, "Load", 64.0f))
        {
            load();
            deselectAllObjects();
            sceneChanged = true;
        }

        DrawToolbarDivider(x);

        if (DrawToolbarButton(x, "+ Cube", 70.0f))
            sceneChanged = InsertToolbarObject(1, false, camera);

        if (DrawToolbarButton(x, "+ Sphere", 78.0f))
            sceneChanged = InsertToolbarObject(2, false, camera);

        if (DrawToolbarButton(x, "+ Cylinder", 86.0f))
            sceneChanged = InsertToolbarObject(3, false, camera);

        if (DrawToolbarButton(x, "+ Light", 72.0f))
            sceneChanged = InsertToolbarObject(2, true, camera);

        DrawToolbarDivider(x);

        if (DrawToolbarButton(x, "Delete", 72.0f))
        {
            deleteobj();
            sceneChanged = true;
        }

        DrawToolbarDivider(x);

        GuiToggle(
            { x, 12.0f, 104.0f, 34.0f },
            "Workspace",
            &workspaceVisible
        );

        x += 110.0f;

        GuiToggle(
            { x, 12.0f, 100.0f, 34.0f },
            "Properties",
            &propertiesVisible
        );
    }

    DrawViewSelector(
        camera,
        currentView,
        lastView,
        viewDropdownOpen,
        cameraLocked
    );

    Rectangle optionsButton = {
        (float)GetScreenWidth() - 46.0f,
        12.0f,
        34.0f,
        34.0f
    };

    if (GuiButton(optionsButton, ""))
    {
        sceneManagerChangeScene(sceneId::SCENE_OPTIONS);
    }

    GuiDrawIcon(
        ICON_GEAR_BIG,
        (int)optionsButton.x,
        (int)optionsButton.y,
        2,
        GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))
    );

    if (sceneChanged)
    {
        workspaceVisible = true;
        propertiesVisible = true;
    }

    return sceneChanged;
}
