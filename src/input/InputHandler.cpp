#include "InputHandler.h"

#include <raylib.h>
#include <raygui.h>
#include <objects/object.h>

Vector2 clickPos = { 0.0f, 0.0f };
Vector3 objPosn = { 0.0f, 0.0f, 0.0f };

void contextMenu(bool& mouseButtonPressed, Camera3D& camera)
{
    // Menu sizes
    const float rootWidth = 150.0f;
    const float subWidth = 160.0f;
    const float itemHeight = 30.0f;

    const int rootCount = 3;
    const int subCount = 3;

    const char* rootMenu[rootCount] = {
        "Insert Mesh",
        "Object Editing",
        "Delete Object"
    };

    const char* meshMenu[subCount] = {
        "Insert Cube",
        "Insert Sphere",
        "Insert Cylinder"
    };

    enum RootMenuIndex {
        MENU_INSERT_MESH = 0,
        MENU_OBJECT_EDITING = 1,
        MENU_DELETE_OBJECT = 2
    };

    // Open menu on right click
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        mouseButtonPressed = true;
        clickPos = GetMousePosition();

        // Large ground plane for placing objects
        float planeSize = 10000.0f;

        Vector3 p1 = { -planeSize, 0.0f, -planeSize };
        Vector3 p2 = {  planeSize, 0.0f, -planeSize };
        Vector3 p3 = {  planeSize, 0.0f,  planeSize };
        Vector3 p4 = { -planeSize, 0.0f,  planeSize };

        Ray ray = GetScreenToWorldRay(clickPos, camera);
        RayCollision collision = GetRayCollisionQuad(ray, p1, p2, p3, p4);

        if (collision.hit)
        {
            objPosn = collision.point;
        }
    }

    if (!mouseButtonPressed)
    {
        return;
    }

    Vector2 mouse = GetMousePosition();

    Rectangle rootBounds = {
        clickPos.x,
        clickPos.y,
        rootWidth,
        itemHeight * rootCount
    };

    Rectangle insertMeshButton = {
        rootBounds.x,
        rootBounds.y + MENU_INSERT_MESH * itemHeight,
        rootBounds.width,
        itemHeight
    };

    Rectangle subBounds = {
        rootBounds.x + rootBounds.width,
        rootBounds.y + MENU_INSERT_MESH * itemHeight,
        subWidth,
        itemHeight * subCount
    };

    bool hoverInsertMesh = CheckCollisionPointRec(mouse, insertMeshButton);
    bool hoverSubMenu = CheckCollisionPointRec(mouse, subBounds);

    // Submenu appears only when hovering "Insert Mesh" or the submenu itself
    bool showSubMenu = hoverInsertMesh || hoverSubMenu;

    // Draw root menu buttons
    for (int i = 0; i < rootCount; i++)
    {
        Rectangle itemRect = {
            rootBounds.x,
            rootBounds.y + i * itemHeight,
            rootBounds.width,
            itemHeight
        };

        if (GuiButton(itemRect, rootMenu[i]))
        {
            if (i == MENU_OBJECT_EDITING)
            {
                TraceLog(LOG_INFO, "Object Editing clicked");

                mouseButtonPressed = false;
                return;
            }
            else if (i == MENU_DELETE_OBJECT)
            {
                TraceLog(LOG_INFO, "Delete Object clicked");

                mouseButtonPressed = false;
                return;
            }

            // Do not close when clicking Insert Mesh.
            // It has a submenu.
        }
    }

    // Draw submenu only on hover
    if (showSubMenu)
    {
        for (int i = 0; i < subCount; i++)
        {
            Rectangle itemRect = {
                subBounds.x,
                subBounds.y + i * itemHeight,
                subBounds.width,
                itemHeight
            };

            if (GuiButton(itemRect, meshMenu[i]))
            {
                if (i == 0)
                {
                    cube(objPosn);
                }
                else if (i == 1)
                {
                    sphere(objPosn);
                }
                else if (i == 2)
                {
                    cylinder(objPosn);
                }

                mouseButtonPressed = false;
                return;
            }
        }
    }

    // Close menu if left-clicked outside root menu and outside submenu
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        bool clickedRoot = CheckCollisionPointRec(mouse, rootBounds);
        bool clickedSub = showSubMenu && CheckCollisionPointRec(mouse, subBounds);

        if (!clickedRoot && !clickedSub)
        {
            mouseButtonPressed = false;
            return;
        }
    }
}