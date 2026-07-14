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

void contextMenu(bool& mouseButtonPressed, Camera3D& camera)
{


    const char** submenuText = NULL;
    int subMenuSize = 0;

    Rectangle menuRec = { 0, 0, 125, 180 };

    // Large ground plane used to convert the right-click ray into a world position.
    float planeSize = 10000.0f;
    Vector3 p1 = { -planeSize, 0.0f, -planeSize };
    Vector3 p2 = { planeSize, 0.0f, -planeSize };
    Vector3 p3 = { planeSize, 0.0f,  planeSize };
    Vector3 p4 = { -planeSize, 0.0f,  planeSize };

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        mouseButtonPressed = true;

        clickPos = GetMousePosition();
        Ray ray = GetScreenToWorldRay(clickPos, camera); // creating an object ray from raylib, that will project a ray from the camera to the coordinates to get a #D position
        RayCollision collision = GetRayCollisionQuad(ray, p1, p2, p3, p4);

        if (collision.hit)
        {
            objPosn = collision.point;
        }

        state = STATE_SHOW_MENU;
    }

    static const char* menuInsertMesh = "Insert Mesh";
    static const char* menuObjectEditing = "Object Editing";
    static const char* menuDeleteObject = "Delete Object";
    static const char* menuAddMaterial = "Add Material";

    static const char* rootMenu[] = {
        menuInsertMesh,
        menuObjectEditing,
        menuDeleteObject,
        menuAddMaterial
    };

    static const char* addCube = "Insert Cube";
    static const char* addSphere = "Insert Sphere";
    static const char* addCylinder = "Insert Cylinder";
    static const char* addPointLight = "Insert Point Light";

    static const char* addMesh[] = {
        addCube,
        addSphere,
        addCylinder,
        addPointLight
    };

	static const char* addConcrete = "Add Concrete";
	static const char* addWood = "Add Wood";
	static const char* addPlastic = "Add Plastic";
	static const char* addCobblestone = "Add Cobblestone";
    static const char* addBrick = "Add Brick";
    static const char* addTiles = "Add Tiles";
    static const char* addMetal = "Add Metal";
    static const char* addMarble= "Add Marble";
    static const char* addAsphalt = "Add Asphalt";

    static const char* addMaterial[] = {
		addConcrete, addWood, addPlastic, addCobblestone, addBrick, addTiles, addMetal, addMarble, addAsphalt
    };
    enum rootMenuIndex
    {
        Menu_InsertMesh = 0,
        Menu_ObjectEditing,
        Menu_DeleteObject,
		Menu_AddMaterial
    };

    if (mouseButtonPressed)
    {
        menuRec.x = clickPos.x;
        menuRec.y = clickPos.y;
    }

    const int itemHeight = GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) + GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING);

    const int rootMenuSize = sizeof(rootMenu) / sizeof(rootMenu[0]);

    float rootTextWidth = 0.0f;
    for (const char* item : rootMenu)
        rootTextWidth = std::max(rootTextWidth, MeasureThemeText(item, static_cast<float>(GuiGetStyle(DEFAULT, TEXT_SIZE))).x);

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
            //mainFocused = focused;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && focused >= 0 && focused < rootMenuSize) {
                mainActive = focused;
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                TraceLog(LOG_INFO, TextFormat("CLICKED >>> %s", rootMenu[focused]));

                if (focused == Menu_ObjectEditing)
                {
                    state = STATE_BASE;
                    mainActive = subActive = -1;
                    mouseButtonPressed = false;
                }
                else if (focused == Menu_DeleteObject)
                {
                    deleteobj();
                    state = STATE_BASE;
                    mainActive = subActive = -1;
                    mouseButtonPressed = false;
                }
            }
        }

        if (mainActive == Menu_InsertMesh)
        {
            submenuText = addMesh;
            state = STATE_SHOW_SUBMENU;
            subMenuSize = sizeof(addMesh) / sizeof(addMesh[0]);
        }
        if (mainActive == Menu_AddMaterial)
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
            submenuTextWidth = std::max(submenuTextWidth, MeasureThemeText(submenuText[i],static_cast<float>(GuiGetStyle(DEFAULT, TEXT_SIZE))).x);
        const float submenuWidth = std::max(165.0f, submenuTextWidth + 36.0f);
        float submenuX = menuRec.x + menuRec.width + 2.0f;
        if (submenuX + submenuWidth > GetScreenWidth() - 4.0f) submenuX = menuRec.x - submenuWidth - 2.0f;

        Rectangle bounds = { submenuX, menuRec.y + (float)mainActive * itemHeight, submenuWidth, (float)subMenuSize * itemHeight + 10 };
        bounds.y = std::clamp(bounds.y, 4.0f, std::max(4.0f, GetScreenHeight() - bounds.height - 4.0f));

        int focused = -1;
		// GuiListViewEx is supposed to return the index of the selected item, but it seems to return 0 atm, so removed the assignment from return value and just used pointer assignment
        GuiListViewEx(bounds, submenuText, subMenuSize, &subScrollIndex, &subActive, &focused);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && focused >= 0 && focused < subMenuSize)
        {
            Vector2 mousePosition = GetMousePosition();
            Rectangle itemRect = { bounds.x, bounds.y + focused * itemHeight, bounds.width, (float)itemHeight };

            if (CheckCollisionPointRec(mousePosition, itemRect))
            {
                if (mainActive == Menu_InsertMesh )
                //if (strcmp(rootMenu[mainActive], "Insert Mesh") == 0)
                {
                    if (strcmp(submenuText[focused], "Insert Cube") == 0)
                    {
                        TraceLog(LOG_INFO, "Old material:");

                        objects.push_back(std::make_unique<cube>(objPosn));
                    }
                    else if (strcmp(submenuText[focused], "Insert Sphere") == 0)
                    {
                        //sphere(objPosn, GRAY);
                    }
                    else if (strcmp(submenuText[focused], "Insert Cylinder") == 0)
                    {
                        //cylinder(objPosn, GRAY);
                    }
                    else if (strcmp(submenuText[focused], "Insert Point Light") == 0)
                    {
                        lights.push_back(std::make_unique<Light>(objPosn));
                    }
                } else
                if (strcmp(rootMenu[mainActive], "Add Material") == 0)
                {
                    TraceLog(LOG_INFO,
                        "mainActive=%d, focused=%d",
                        mainActive,
                        focused
                    );
                    if (strcmp(submenuText[focused], "Add Concrete") == 0)
                    {
                        TraceLog(LOG_INFO, "outside selected  Old material:");

                        for (shape* object : selectedObjects)
                        {
                            TraceLog(LOG_INFO, "Old material:");

                            object->applyMaterial(MATERIAL_CONCRETE);
                        }
                    }
                    else if (strcmp(submenuText[focused], "Add Wood") == 0)
                    {
                        for (shape* object : selectedObjects)
                        {
                            object->applyMaterial(MATERIAL_WOOD);
                        }
                    }
                    else if (strcmp(submenuText[focused], "Add Plastic") == 0)
                    {
                        for (shape* object : selectedObjects)
                        {
                            object->applyMaterial(MATERIAL_PLASTIC);
                        }
                    }
                    else if (strcmp(submenuText[focused], "Add Cobblestone") == 0)
                    {
                        for (shape* object : selectedObjects)
                        {
                            object->applyMaterial(MATERIAL_COBBLESTONE);
                        }
                    }
                    else if (strcmp(submenuText[focused], "Add Tiles") == 0)
                    {
                        for (shape* object : selectedObjects)
                        {
                            object->applyMaterial(MATERIAL_TILES);
                        }
                    }
                    else if (strcmp(submenuText[focused], "Add Metal") == 0)
                    {
                        for (shape* object : selectedObjects)
                        {
                            object->applyMaterial(MATERIAL_METAL);
                        }
                    }
                    else if (strcmp(submenuText[focused], "Add Marble") == 0)
                    {
                        for (shape* object : selectedObjects)
                        {
                            object->applyMaterial(MATERIAL_MARBLE);
                        }
                    }
                    else if (strcmp(submenuText[focused], "Add Asphalt") == 0)
                    {
                        for (shape* object : selectedObjects)
                        {
                            object->applyMaterial(MATERIAL_ASPHALT);
                        }
                    }
                    else if (strcmp(submenuText[focused], "Add Brick") == 0)
                    {
                        for (shape* object : selectedObjects)
                        {
                            object->applyMaterial(MATERIAL_BRICK);
                        }
                    }
                }
            }

            state = STATE_BASE;
            mainActive = subActive = -1;
            mouseButtonPressed = false;
        }
    }
}
