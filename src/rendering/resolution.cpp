#include "resolution.h"
#include "raylib.h"
#include "raygui.h"

// Needed because btnWidth, btnHeight, btnLeft, btnTop, btnGap
// are declared extern in sceneManager.h
#include "ui/scenes/sceneManager.h"

const resolutionClass resolutions[5] = {
    {1280, 720, "720p (HD)" },
    {1600, 900, "900p (HD+)" },
    {1920, 1080, "1080p (Full HD)" },
    {2560, 1440, "1440p (Quad HD)" },
    {2560, 1600, "1600p (WQXGA)" }
};

void changeButtonResolution() {
    btnWidth  = GetScreenWidth() * 0.16f;
    btnHeight = GetScreenHeight() * 0.07f;
    btnLeft   = GetScreenWidth() * 0.08f;
    btnTop    = GetScreenHeight() * 0.14f;
    btnGap    = btnHeight * 1.25f;
}

void topBar(int& activeRes, bool& dropdownEditMode) {
    const char* options =
        "720p (HD);900p (HD+);1080p (Full HD);1440p (Quad HD);1600p (WQXGA)";

    int oldRes = activeRes;

    if (GuiDropdownBox(Rectangle{ 10, 5, 200, 30 }, options, &activeRes, dropdownEditMode)) {
        dropdownEditMode = !dropdownEditMode;
    }

    if (activeRes != oldRes) {
        resolutionClass selected = resolutions[activeRes];

        SetWindowSize(selected.width, selected.height);

        changeButtonResolution();
    }
}