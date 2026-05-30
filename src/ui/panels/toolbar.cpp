#include "toolbar.h"
#include <raylib.h>
#include <raygui.h>
const res cr[5] = {
    {1280, 720, "720p (HD)" },
    {1600, 900, "900p (HD+)" },
    {1920, 1080, "1080p (Full HD)" },
    {2560, 1440, "1440p (Quad HD)" },
    {2560, 1600, "1600p (WQXGA)" }
};
void topbar(int &activeRes, bool &dropdownEditMode) {
    res r = cr[0]; 
    const char* options = "720p (HD);900p (HD+);1080p (Full HD);1440p (Quad HD);1600p (WQXGA)";
    if (GuiDropdownBox(Rectangle{ 10, 5, 200, 30 }, options, &activeRes, dropdownEditMode)) {
        dropdownEditMode = !dropdownEditMode; 
    }
}