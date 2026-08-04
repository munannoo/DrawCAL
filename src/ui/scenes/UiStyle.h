#ifndef UI_STYLE_H
#define UI_STYLE_H

#include "raylib.h"
#include "raygui.h"
#include <algorithm>

namespace UiStyle {
    inline const Color BackgroundColor = { 70, 75, 80, 255 };
    inline const Color PanelColor = { 0, 0, 0, 0 };
    inline const Color PanelBorderColor = { 0, 0, 0, 0 };
    inline const Color TitleColor = { 235, 239, 244, 255 };
    inline const Color SubtitleColor = { 190, 198, 208, 255 };
    inline const Color ButtonTextColor = { 235, 239, 244, 255 };

    inline const float PanelRoundness = 0.16f;
    inline const int PanelSegments = 6;

    inline float GetUiScale() {
        const float referenceHeight = 1080.0f;
        return std::clamp(GetScreenHeight() / referenceHeight, 0.6f, 2.0f);
    }

    inline void LoadMinimalStyle() {
        GuiLoadStyleDefault();

        float scale = GetUiScale();

        GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(BackgroundColor));
        GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, ColorToInt(PanelBorderColor));
        GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(Color{ 255, 255, 255, 10 }));
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(ButtonTextColor));
        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, ColorToInt(Color{ 255, 255, 255, 32 }));
        GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, ColorToInt(Color{ 255, 255, 255, 16 }));
        GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt(ButtonTextColor));
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, ColorToInt(Color{ 255, 255, 255, 32 }));
        GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, ColorToInt(Color{ 255, 255, 255, 24 }));
        GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, ColorToInt(ButtonTextColor));
        GuiSetStyle(DEFAULT, BORDER_WIDTH, std::max(1, (int)(1 * scale)));
        GuiSetStyle(DEFAULT, TEXT_SIZE, (int)(28 * scale));
        GuiSetStyle(DEFAULT, TEXT_SPACING, (int)(4 * scale));
    }

    inline void DrawBackground() {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BackgroundColor);
    }

    inline void DrawPanel(const Rectangle& /*panel*/) {}

    inline Rectangle CreatePanel(float btnLeft, float btnTop, float btnWidth, float btnHeight, float btnGap, int buttonCount) {
        const float panelWidth = btnWidth + 72.0f;
        const float panelHeight = btnHeight * buttonCount + btnGap * (buttonCount - 1) + 104.0f;
        return Rectangle{ btnLeft - 36.0f, btnTop - 72.0f, panelWidth, panelHeight };
    }

    inline float GetHeaderHeight(bool hasSubtitle = true) {
        float scale = GetUiScale();
        const int titleFontSize = (int)(48 * scale);
        const int subtitleFontSize = (int)(18 * scale);
        const float titleSubtitleGap = 12.0f * scale;

        float height = (float)titleFontSize;
        if (hasSubtitle) height += titleSubtitleGap + (float)subtitleFontSize;
        return height;
    }

    inline void ComputeHeaderLayout(bool hasSubtitle, float& outHeaderTop, float& outContentTop) {
        float scale = GetUiScale();
        outHeaderTop = 60.0f * scale;
        outContentTop = outHeaderTop + GetHeaderHeight(hasSubtitle) + 50.0f * scale;
    }

    inline void DrawSceneHeader(const char* title, const char* subtitle, float headerTop) {
        float scale = GetUiScale();
        const int titleFontSize = (int)(48 * scale);
        const int subtitleFontSize = (int)(18 * scale);
        const float titleSubtitleGap = 12.0f * scale;

        Vector2 titleSize = MeasureTextEx(GetFontDefault(), title, titleFontSize, 1);
        DrawText(title, (int)((GetScreenWidth() - titleSize.x) * 0.5f), (int)headerTop, titleFontSize, TitleColor);

        if (subtitle && subtitle[0] != '\0') {
            float subtitleY = headerTop + titleFontSize + titleSubtitleGap;
            Vector2 subtitleSize = MeasureTextEx(GetFontDefault(), subtitle, subtitleFontSize, 1);
            DrawText(subtitle, (int)((GetScreenWidth() - subtitleSize.x) * 0.5f), (int)subtitleY, subtitleFontSize, SubtitleColor);
        }
    }
}

#endif // UI_STYLE_H