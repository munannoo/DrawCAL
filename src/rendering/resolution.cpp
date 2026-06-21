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
     btnWidth = int(GetScreenWidth() * 0.16f);   // 16% of width
     btnHeight = int(GetScreenHeight() * 0.07f);   // 7% of height
     btnLeft = int(GetScreenWidth() * 0.08f);  // 8% margin from left
     btnTop = int(GetScreenHeight() * 0.14f);  // starting top
     btnGap = int(btnHeight * 1.25f);
}

