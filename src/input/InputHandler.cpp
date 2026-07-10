#include "InputHandler.h"

#include <raygui.h>
#include <objects/object.h>
#include <cstring>
#include <raylib.h>

static Vector2 clickPos = { 0.0f, 0.0f };
static Vector3 objPosn = { 0.0f, 0.0f, 0.0f };

void contextMenu(bool& mouseButtonPressed, Camera3D& camera)
{
    static int mainFocused = -1;
    static int mainActive = -1;
    static int subActive = -1;
    static int state = STATE_BASE;

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

    static const char* rootMenu[] = {
        menuInsertMesh,
        menuObjectEditing,
        menuDeleteObject
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

    enum rootMenuIndex
    {
        Menu_InsertMesh = 0,
        Menu_ObjectEditing,
        Menu_DeleteObject
    };

    if (mouseButtonPressed)
    {
        menuRec.x = clickPos.x;
        menuRec.y = clickPos.y;
    }

    const int itemHeight =
        GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT) +
        GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING);

    const int rootMenuSize = sizeof(rootMenu) / sizeof(rootMenu[0]);

    if (state == STATE_SHOW_MENU || state == STATE_SHOW_SUBMENU)
    {
        menuRec.height = itemHeight * rootMenuSize + 10;

        int focused = mainFocused;
        mainActive = GuiListViewEx(menuRec, rootMenu, rootMenuSize, NULL, &mainActive, &focused);

        if (focused >= 0 && focused < rootMenuSize)
        {
            mainFocused = focused;
            mainActive = focused;

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                TraceLog(LOG_INFO, TextFormat("CLICKED >>> %s", rootMenu[focused]));

                if (focused == Menu_ObjectEditing)
                {
                    state = STATE_BASE;
                    mainFocused = mainActive = subActive = -1;
                    mouseButtonPressed = false;
                }
                else if (focused == Menu_DeleteObject)
                {
                    deleteobj();
                    state = STATE_BASE;
                    mainFocused = mainActive = subActive = -1;
                    mouseButtonPressed = false;
                }
            }
        }

        if (mainFocused == Menu_InsertMesh)
        {
            submenuText = addMesh;
            state = STATE_SHOW_SUBMENU;
            subMenuSize = sizeof(addMesh) / sizeof(addMesh[0]);
        }
    }

    if (state == STATE_SHOW_SUBMENU && submenuText != NULL)
    {
        Rectangle bounds = {
            menuRec.x + menuRec.width + 2,
            menuRec.y + (float)mainFocused * itemHeight,
            130,
            (float)subMenuSize * itemHeight + 10
        };

        int focused = -1;
        subActive = GuiListViewEx(bounds, submenuText, subMenuSize, NULL, &subActive, &focused);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && focused >= 0 && focused < subMenuSize)
        {
            Vector2 mousePosition = GetMousePosition();
            Rectangle itemRect = {
                bounds.x,
                bounds.y + focused * itemHeight,
                bounds.width,
                (float)itemHeight
            };

            if (CheckCollisionPointRec(mousePosition, itemRect))
            {
                if (strcmp(rootMenu[mainActive], "Insert Mesh") == 0)
                {
                    if (strcmp(submenuText[focused], "Insert Cube") == 0)
                    {
                        cube(objPosn, GRAY);
                    }
                    else if (strcmp(submenuText[focused], "Insert Sphere") == 0)
                    {
                        sphere(objPosn, GRAY);
                    }
                    else if (strcmp(submenuText[focused], "Insert Cylinder") == 0)
                    {
                        cylinder(objPosn, GRAY);
                    }
                    else if (strcmp(submenuText[focused], "Insert Point Light") == 0)
                    {
                        lightSphere(objPosn, WHITE);
                    }
                }
            }

            state = STATE_BASE;
            mainFocused = mainActive = subActive = -1;
            mouseButtonPressed = false;
        }
    }
}
