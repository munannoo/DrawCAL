#include "InputHandler.h"

#include <raygui.h>
#include <objects/object.h>
#include <cstring>
#include <raylib.h>
#include "ui/themes/themes.h"
#include <algorithm>

static Vector2 clickPos = { 0.0f, 0.0f };
static Vector3 objPosn = { 0.0f, 0.0f, 0.0f };

// focused = if hovered
//static int mainFocused = -1;
// active = if selected
static int mainActive = -1;
// subFocused is declared locally
static int subActive = -1;
static int scrollIndex = -1;
static int subScrollIndex = -1;
static int state = STATE_BASE;

void contextMenu(bool& mouseButtonPressed, Camera3D& camera, Rectangle viewport, bool guidedRestricted)
{
    const char** submenuText = NULL;
    int subMenuSize = 0;

    Rectangle menuRec = { 0, 0, 125, 180 };

    Rectangle submenuBounds = { 0 };
    bool submenuVisible = false;

    float planeSize = 10000.0f;
    Vector3 p1 = { -planeSize, 0.0f, -planeSize };
    Vector3 p2 = { planeSize, 0.0f, -planeSize };
    Vector3 p3 = { planeSize, 0.0f,  planeSize };
    Vector3 p4 = { -planeSize, 0.0f,  planeSize };

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        mouseButtonPressed = true;

        clickPos = GetMousePosition();
        Vector2 localClick = { clickPos.x - viewport.x, clickPos.y - viewport.y };
        Ray ray = GetScreenToWorldRayEx(localClick, camera, viewport.width, viewport.height);
        RayCollision collision = GetRayCollisionQuad(ray, p1, p2, p3, p4);

        if (collision.hit)
        {
            objPosn = collision.point;
        }

        state = STATE_SHOW_MENU;
    }

    static const char* menuInsertMesh = "Insert Mesh";
    static const char* menuDeleteObject = "Delete Object";
    static const char* menuAddMaterial = "Add Material";

    enum rootMenuIndex
    {
        Menu_InsertMesh = 0,
        Menu_DeleteObject,
        Menu_AddMaterial
    };

    static const char* fullRootMenu[] = { menuInsertMesh, menuDeleteObject, menuAddMaterial };
    static const rootMenuIndex fullRootMenuIds[] = { Menu_InsertMesh, Menu_DeleteObject, Menu_AddMaterial };

    static const char* restrictedRootMenu[] = { menuInsertMesh, menuAddMaterial };
    static const rootMenuIndex restrictedRootMenuIds[] = { Menu_InsertMesh, Menu_AddMaterial };

    const char** rootMenu = guidedRestricted ? restrictedRootMenu : fullRootMenu;
    const rootMenuIndex* rootMenuIds = guidedRestricted ? restrictedRootMenuIds : fullRootMenuIds;
    const int rootMenuSize = guidedRestricted
        ? (sizeof(restrictedRootMenu) / sizeof(restrictedRootMenu[0]))
        : (sizeof(fullRootMenu) / sizeof(fullRootMenu[0]));

    static const char* addCube = "Insert Cube";
    static const char* addSphere = "Insert Sphere";
    static const char* addCylinder = "Insert Cylinder";
    static const char* addPointLight = "Insert Point Light";

    static const char* fullAddMesh[] = { addCube, addSphere, addCylinder, addPointLight };
    static const char* restrictedAddMesh[] = { addPointLight }; // guided mode: light only, no new geometry

    static const char* addConcrete = "Add Concrete";
    static const char* addWood = "Add Wood";
    static const char* addPlastic = "Add Plastic";
    static const char* addCobblestone = "Add Cobblestone";
    static const char* addBrick = "Add Brick";
    static const char* addTiles = "Add Tiles";
    static const char* addMetal = "Add Metal";
    static const char* addMarble = "Add Marble";
    static const char* addAsphalt = "Add Asphalt";

    static const char* addMaterial[] = {
        addConcrete, addWood, addPlastic, addCobblestone, addBrick, addTiles, addMetal, addMarble, addAsphalt
    };

    if (mouseButtonPressed)
    {
        menuRec.x = clickPos.x;
        menuRec.y = clickPos.y;
    }

    const int itemHeight = GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING);

    float rootTextWidth = 0.0f;
    for (int i = 0; i < rootMenuSize; ++i)
        rootTextWidth = std::max(rootTextWidth, MeasureThemeText(rootMenu[i], static_cast<float>(GuiGetStyle(DEFAULT, TEXT_SIZE))).x);

    menuRec.width = std::max(165.0f, rootTextWidth + 36.0f);
    menuRec.height = static_cast<float>(itemHeight * rootMenuSize + 10);
    menuRec.x = std::clamp(menuRec.x, 4.0f, std::max(4.0f, GetScreenWidth() - menuRec.width - 4.0f));
    menuRec.y = std::clamp(menuRec.y, 4.0f, std::max(4.0f, GetScreenHeight() - menuRec.height - 4.0f));

    // bloc: root menu
    if (state == STATE_SHOW_MENU || state == STATE_SHOW_SUBMENU)
    {
        menuRec.height = itemHeight * rootMenuSize + 10;

        int focused = -1;
        GuiListViewEx(menuRec, rootMenu, rootMenuSize, &scrollIndex, &mainActive, &focused);

        if (focused >= 0 && focused < rootMenuSize)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && focused >= 0 && focused < rootMenuSize) {
                mainActive = focused;
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                TraceLog(LOG_INFO, TextFormat("CLICKED >>> %s", rootMenu[focused]));

                rootMenuIndex focusedId = rootMenuIds[focused];
                if (focusedId == Menu_DeleteObject)
                {
                    deleteObjects();
                    state = STATE_BASE;
                    mainActive = subActive = -1;
                    mouseButtonPressed = false;
                }
            }
        }

        rootMenuIndex activeId = (mainActive >= 0 && mainActive < rootMenuSize) ? rootMenuIds[mainActive] : Menu_InsertMesh;

        if (mainActive >= 0 && activeId == Menu_InsertMesh)
        {
            submenuText = guidedRestricted ? restrictedAddMesh : fullAddMesh;
            state = STATE_SHOW_SUBMENU;
            subMenuSize = guidedRestricted
                ? (sizeof(restrictedAddMesh) / sizeof(restrictedAddMesh[0]))
                : (sizeof(fullAddMesh) / sizeof(fullAddMesh[0]));
        }
        if (mainActive >= 0 && activeId == Menu_AddMaterial)
        {
            submenuText = addMaterial;
            state = STATE_SHOW_SUBMENU;
            subMenuSize = sizeof(addMaterial) / sizeof(addMaterial[0]);
        }
    }

    // bloc: submenu
    if (state == STATE_SHOW_SUBMENU && submenuText != NULL)
    {
        float submenuTextWidth = 0.0f;
        for (int i = 0; i < subMenuSize; ++i)
            submenuTextWidth = std::max(submenuTextWidth, MeasureThemeText(submenuText[i], static_cast<float>(GuiGetStyle(DEFAULT, TEXT_SIZE))).x);
        const float submenuWidth = std::max(165.0f, submenuTextWidth + 36.0f);
        float submenuX = menuRec.x + menuRec.width + 2.0f;
        if (submenuX + submenuWidth > GetScreenWidth() - 4.0f) submenuX = menuRec.x - submenuWidth - 2.0f;

        Rectangle bounds = { submenuX, menuRec.y + (float)mainActive * itemHeight, submenuWidth, (float)subMenuSize * itemHeight + 10 };
        bounds.y = std::clamp(bounds.y, 4.0f, std::max(4.0f, GetScreenHeight() - bounds.height - 4.0f));

        submenuBounds = bounds;
        submenuVisible = true;

        int focused = -1;
        GuiListViewEx(bounds, submenuText, subMenuSize, &subScrollIndex, &subActive, &focused);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && focused >= 0 && focused < subMenuSize)
        {
            Vector2 mousePosition = GetMousePosition();
            Rectangle itemRect = { bounds.x, bounds.y + focused * itemHeight, bounds.width, (float)itemHeight };

            if (CheckCollisionPointRec(mousePosition, itemRect))
            {
                rootMenuIndex activeId = (mainActive >= 0 && mainActive < rootMenuSize) ? rootMenuIds[mainActive] : Menu_InsertMesh;

                if (activeId == Menu_InsertMesh)
                {
                    if (strcmp(submenuText[focused], "Insert Cube") == 0)
                    {
                        objects.push_back(std::make_unique<cube>(objPosn));
                    }
                    else if (strcmp(submenuText[focused], "Insert Sphere") == 0)
                    {
                        objects.push_back(std::make_unique<sphere>(objPosn));
                    }
                    else if (strcmp(submenuText[focused], "Insert Cylinder") == 0)
                    {
                        objects.push_back(std::make_unique<cylinder>(objPosn));
                    }
                    else if (strcmp(submenuText[focused], "Insert Point Light") == 0)
                    {
                        lights.push_back(std::make_unique<Light>(objPosn));
                    }
                }
                else if (activeId == Menu_AddMaterial)
                {
                    if (strcmp(submenuText[focused], "Add Concrete") == 0)
                    {
                        for (shape* object : selectedObjects) object->applyMaterial(MATERIAL_CONCRETE);
                    }
                    else if (strcmp(submenuText[focused], "Add Wood") == 0)
                    {
                        for (shape* object : selectedObjects) object->applyMaterial(MATERIAL_WOOD);
                    }
                    else if (strcmp(submenuText[focused], "Add Plastic") == 0)
                    {
                        for (shape* object : selectedObjects) object->applyMaterial(MATERIAL_PLASTIC);
                    }
                    else if (strcmp(submenuText[focused], "Add Cobblestone") == 0)
                    {
                        for (shape* object : selectedObjects) object->applyMaterial(MATERIAL_COBBLESTONE);
                    }
                    else if (strcmp(submenuText[focused], "Add Tiles") == 0)
                    {
                        for (shape* object : selectedObjects) object->applyMaterial(MATERIAL_TILES);
                    }
                    else if (strcmp(submenuText[focused], "Add Metal") == 0)
                    {
                        for (shape* object : selectedObjects) object->applyMaterial(MATERIAL_METAL);
                    }
                    else if (strcmp(submenuText[focused], "Add Marble") == 0)
                    {
                        for (shape* object : selectedObjects) object->applyMaterial(MATERIAL_MARBLE);
                    }
                    else if (strcmp(submenuText[focused], "Add Asphalt") == 0)
                    {
                        for (shape* object : selectedObjects) object->applyMaterial(MATERIAL_ASPHALT);
                    }
                    else if (strcmp(submenuText[focused], "Add Brick") == 0)
                    {
                        for (shape* object : selectedObjects) object->applyMaterial(MATERIAL_BRICK);
                    }
                }
            }

            state = STATE_BASE;
            mainActive = subActive = -1;
            mouseButtonPressed = false;
        }
    }

    if (state != STATE_BASE && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        Vector2 mousePosition = GetMousePosition();
        bool insideRoot = CheckCollisionPointRec(mousePosition, menuRec);
        bool insideSub = submenuVisible && CheckCollisionPointRec(mousePosition, submenuBounds);

        if (!insideRoot && !insideSub)
        {
            state = STATE_BASE;
            mainActive = subActive = -1;
            mouseButtonPressed = false;
        }
    }
}