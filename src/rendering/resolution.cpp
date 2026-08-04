#include "resolution.h"
#include "raylib.h"
#include "raygui.h"
#include "ui/scenes/UiStyle.h"

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
    float screenW = static_cast<float>(GetScreenWidth());
    float screenH = static_cast<float>(GetScreenHeight());

    btnWidth = screenW * 0.26f;
    btnHeight = screenH * 0.075f;
    btnGap = btnHeight * 1.5f;
    btnLeft = (screenW - btnWidth) * 0.5f;

    const float totalButtonHeight = btnHeight * 5.0f + btnGap * 4.0f;
    const float contentHeight = totalButtonHeight + 100.0f;

    float contentTop;
    UiStyle::ComputeHeaderLayout(true, headerTop, contentTop);
    btnTop = contentTop; // guaranteed clear of the header, at any scale

}

