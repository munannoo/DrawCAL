    #include "InputHandler.h"
    #include "../ui/widgets/buttons.h"
    #include  <raygui.h>
    #include <objects/object.h>
    #include <cstring>
    #include <raylib.h>
    extern Camera3D camera;
    extern Model model;
    static Vector2 clickPos = { 0.0f, 0.0f };
    static Vector3 objPosn = { 0.0f, 0.0f, 0.0f };

    void loadTexture() {
    Texture2D texture = LoadTexture("../assets/textures/Metal_texture.png");
    // if (texture.id == 0) {
    //     std::cerr << "Failed to load texture: " << "../assets/textures/assets/textures/Metal_texture.png" << std::endl;
    // }
    Model model = LoadModelFromMesh(GenMeshPlane(2,2,4,3));
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    Vector3 position = { 10.0f, 0.0f, 0.0f };
    DrawModel(model, position, 1.0f, WHITE);
}

    void contextMenu(bool& mouseButtonPressed,Camera3D& camera)
    {   
        static int mainFocused = -1;
        static int mainActive = -1;

        static int subActive = -1;
        char** submenuText = NULL;
        int sz = 0; // size of submenu, used to calculate height of submenu box
        static int state = STATE_BASE;
        Rectangle menuRec = {0, 0, 100, 180};

        // We will use a large plane to get the position of the ray collision, as we want to place objects on the "ground"
        float planeSize = 10000.0f;
        Vector3 p1 = { -planeSize, 0.0f, -planeSize };
        Vector3 p2 = {  planeSize, 0.0f, -planeSize };
        Vector3 p3 = {  planeSize, 0.0f,  planeSize };
        Vector3 p4 = { -planeSize, 0.0f,  planeSize };

        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
            mouseButtonPressed=true;
            
            clickPos = GetMousePosition();
            Ray ray = GetMouseRay(clickPos, camera); // creating an object ray from raylib, that will project a ray from the camera to the coordinates to get a #D position
            RayCollision collision = GetRayCollisionQuad(ray, p1, p2, p3, p4);
            if(collision.hit){
                objPosn = collision.point;
            }
            state = STATE_SHOW_MENU;

        }
        
        // cout << "Hit at: " << objPosn.x << ", " << objPosn.y << ", " << objPosn.z << endl;
        
        char *rootMenu[] = {
            "Insert Mesh",
            "Object Editing",
            "Delete Object",
            "Add Texture"
        };    
        char* addMesh[] = {
            "Insert Cube", 
            "Insert Sphere", 
            "Insert Cylinder"
        };
        enum rootMenuIndex {
            Menu_InsertMesh = 0,
            Menu_ObjectEditing,
            Menu_DeleteObject,
            Add_texture,
        };




        if(mouseButtonPressed)
        {
            menuRec.x = clickPos.x;
            menuRec.y = clickPos.y;
        }
        
        const int itemHeight = (GuiGetStyle(LISTVIEW, LIST_ITEMS_HEIGHT)+GuiGetStyle(LISTVIEW, LIST_ITEMS_SPACING));
        if(state == STATE_SHOW_MENU || state == STATE_SHOW_SUBMENU )
        {
            menuRec.height = itemHeight * (sizeof(rootMenu)/sizeof(rootMenu[0])) + 10; // calculate menu height based on how many items it has
            int focused = mainFocused;
            mainActive = GuiListViewEx(menuRec, rootMenu, (sizeof(rootMenu)/sizeof(rootMenu[0])), NULL, &mainActive, &focused);
            if(focused >= 0 && focused < (int)(sizeof(rootMenu)/sizeof(rootMenu[0])))  { 
                mainFocused = mainActive = focused;
                if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    TraceLog(LOG_INFO, TextFormat("CLICKED >>> %s", rootMenu[focused])); 
                    state = STATE_BASE;
                    mainFocused = mainActive = subActive = -1;
                }
            }
            if(mainFocused == Menu_InsertMesh) { submenuText = addMesh; state = STATE_SHOW_SUBMENU; sz = sizeof(addMesh)/sizeof(addMesh[0]); }
            // else if(mainFocused == Menu_ObjectEditing) { submenuText = (const char**)submenuOpen; state = STATE_SHOW_SUBMENU; sz = ARRAY_SIZE(submenuOpen); }
            // else if(mainFocused == Menu_DeleteObject) { submenuText = (const char**)submenuSaveAs; state = STATE_SHOW_SUBMENU; sz = ARRAY_SIZE(submenuSaveAs); }
        }
        
        if(state == STATE_SHOW_SUBMENU && submenuText != NULL)
        {
            Rectangle bounds = {menuRec.x + menuRec.width + 2, menuRec.y + (float)mainFocused*itemHeight, 100, (float)sz*itemHeight + 10};
            int focused = -1;
            subActive = GuiListViewEx(bounds, submenuText, sz, NULL, &subActive, &focused);
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { 
                if (strcmp(rootMenu[mainActive], "Insert Mesh") == 0)
                {
                    if (strcmp(submenuText[focused], "Insert Cube") == 0)
                    {
                        cube(objPosn);
                    }
                    else if (strcmp(submenuText[focused], "Insert Sphere") == 0)
                    {
                        sphere(objPosn);
                    }
                    else if (strcmp(submenuText[focused], "Insert Cylinder") == 0)
                    {
                        cylinder(objPosn);
                    }
                }
                else if (strcmp(rootMenu[mainActive], "Add Texture") == 0)
                {
                    loadTexture();
                }
                
                
                TraceLog(LOG_INFO, TextFormat("CLICKED >>> %s > %s", rootMenu[mainActive], submenuText[focused])); 
                state = STATE_BASE;
                mainFocused = mainActive = subActive = -1;
            }
        }

        // if(mouseButtonPressed){
        //     DrawRectangle( clickPos.x,  clickPos.y, 150, 150, GRAY); 
        //     Rectangle b1 = { clickPos.x, clickPos.y, 150, 50 };
        //     Rectangle b2 = { clickPos.x, clickPos.y+50, 150, 50 };
        //     Rectangle b3 = { clickPos.x, clickPos.y+100, 150, 50 };
        //     if(GuiButton(b1, "Insert Cube")){
        //         mouseButtonPressed=false;
        //         DrawRectangle( clickPos.x,  clickPos.y, 150, 50, DARKGRAY); 
        //         cube(objPosn);
        //     }
        //     if(GuiButton(b2, "Insert Sphere")){
        //         mouseButtonPressed=false;
        //         DrawRectangle( clickPos.x,  clickPos.y, 150, 50, DARKGRAY); 
        //         sphere(objPosn);
        //     }
        //     if(GuiButton(b3, "Insert Cylinder")){
        //         mouseButtonPressed=false;
        //         DrawRectangle( clickPos.x,  clickPos.y, 150, 50, DARKGRAY); 
        //         cylinder(objPosn);
        //     }
        // }
        
    }
    
