#include "FreeDrawMode.h"
#include "raylib.h"
#include "raygui.h"
#include "raymath.h"
#include <vector>
#include <memory>

namespace UiStyle
{
    constexpr float kPanelPadding = 12.0f;
    constexpr float kSectionGap = 16.0f;
    constexpr float kLabelToField = 6.0f;
    constexpr float kFieldGap = 10.0f;
    constexpr float kDividerMargin = 8.0f;
    constexpr float kPanelHeaderHeight = 34.0f;
    constexpr float kCameraWeight = 0.25f;
    constexpr float kWorkspaceWeight = 0.20f;
    constexpr float kPropertiesWeight = 0.55f;

    const Color kAccent = { 90, 160, 255, 255 };
    const Color kAccentSoft = { 90, 160, 255, 40 };
    const Color kShadow = { 0, 0, 0, 35 };
}

static bool guidedWorkspace = false;
static bool guidedDimensionsVisible = false;

void SetGuidedWorkspace(bool guided)
{
    guidedWorkspace = guided;
    if (guided)
    {
        freeDrawState.mouseButtonPressed = false;
        guidedDimensionsVisible = false;
    }
}

namespace
{
    static Rectangle getEditorDockBounds()
    {
        const float width = Clamp(GetScreenWidth() * 0.30f, 370.0f, 430.0f);
        return { static_cast<float>(GetScreenWidth()) - width, 52.0f, width, static_cast<float>(GetScreenHeight()) - 52.0f };
    }

    const float fontSize = static_cast<float>(GuiGetStyle(DEFAULT, TEXT_SIZE));
    const float spacing = static_cast<float>(std::max(0, GuiGetStyle(DEFAULT, TEXT_SPACING)));
    const float editorControlHeight = std::max(27, GuiGetStyle(DEFAULT, TEXT_SIZE) + 11);

    static RenderTexture2D guidedViewTexture = {};
    static int guidedViewTextureWidth = 0;
    static int guidedViewTextureHeight = 0;

    struct GuidedObjectFrame
    {
        Vector3 center = { 0.0f, 0.0f, 0.0f };
        Vector3 halfExtent = { 1.0f, 1.0f, 1.0f };
    };

    static GuidedObjectFrame GetGuidedObjectFrame()
    {
        shape* target = nullptr;

        // Prefer the actively selected object.
        for (auto& objectPtr : objects)
        {
            if (objectPtr && objectPtr->getSelected())
            {
                target = objectPtr.get();
                break;
            }
        }

        // Imported scenes fall back to the first object in the workspace.
        if (target == nullptr && !objects.empty())
            target = objects.front().get();

        if (target == nullptr) return {};

        const Transform& t = target->getTransform();

        const Vector3 scale = { std::fabs(t.scale.x), std::fabs(t.scale.y), std::fabs(t.scale.z) };

        // transform.rotation is a quaternion (see shape::getMatrix()), not Euler degrees,
        // so we derive the rotation matrix directly from it rather than via MatrixRotateXYZ.
        const Matrix rotation = QuaternionToMatrix(t.rotation);

        GuidedObjectFrame frame;
        frame.center = t.translation;
        // Rotated axis-aligned extents. All built-in meshes have unit local half-extents.
        frame.halfExtent =
        {
            std::fabs(rotation.m0) * scale.x + std::fabs(rotation.m4) * scale.y + std::fabs(rotation.m8) * scale.z,
            std::fabs(rotation.m1) * scale.x + std::fabs(rotation.m5) * scale.y + std::fabs(rotation.m9) * scale.z,
            std::fabs(rotation.m2) * scale.x + std::fabs(rotation.m6) * scale.y + std::fabs(rotation.m10) * scale.z
        };
        return frame;
    }

    static Camera3D MakeGuidedReferenceCamera(Vector3 direction, Vector3 up,
        Vector3 target, float frameHeight,
        float distance)
    {
        Camera3D camera = {};
        camera.position = Vector3Add(target, Vector3Scale(direction, distance));
        camera.target = target;
        camera.up = up;
        camera.fovy = std::max(frameHeight, 0.25f);
        camera.projection = CAMERA_ORTHOGRAPHIC;
        return camera;
    }

    static void EnsureGuidedViewTexture(int width, int height)
    {
        width = std::max(width, 1);
        height = std::max(height, 1);
        if (guidedViewTexture.id != 0 &&
            guidedViewTextureWidth == width && guidedViewTextureHeight == height)
            return;

        if (guidedViewTexture.id != 0) UnloadRenderTexture(guidedViewTexture);
        guidedViewTexture = LoadRenderTexture(width, height);
        guidedViewTextureWidth = width;
        guidedViewTextureHeight = height;
    }

    static void DrawGuidedReferenceView(Rectangle bounds, const char* title,
        const Camera3D& camera,
        float horizontalDimension,
        float verticalDimension)
    {
        const int headerHeight = 30;
        const int contentWidth = std::max(1, static_cast<int>(bounds.width) - 2);
        const int contentHeight = std::max(1, static_cast<int>(bounds.height) - headerHeight - 1);
        EnsureGuidedViewTexture(contentWidth, contentHeight);

        Rectangle localViewport = { 0, 0, static_cast<float>(contentWidth), static_cast<float>(contentHeight) };
        RenderCameraSceneToTexture(camera, localViewport, guidedViewTexture);

        DrawRectangleRec(bounds, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        DrawTexturePro(guidedViewTexture.texture,
            { 0.0f, 0.0f, static_cast<float>(contentWidth),
              -static_cast<float>(contentHeight) },
            { bounds.x + 1.0f, bounds.y + headerHeight,
              bounds.width - 2.0f, bounds.height - headerHeight - 1.0f },
            { 0.0f, 0.0f }, 0.0f, WHITE);
        DrawRectangle(static_cast<int>(bounds.x), static_cast<int>(bounds.y),
            static_cast<int>(bounds.width), headerHeight,
            GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)));
        DrawRectangleLinesEx(bounds, 1.0f,
            GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));
        DrawTextEx(GuiGetFont(), title, { bounds.x + 10.0f, bounds.y + 6.0f }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

        if (!guidedDimensionsVisible) return;

        const Color dimensionColor = Color{ 255, 196, 64, 255 };
        const Rectangle content = { bounds.x + 1.0f, bounds.y + headerHeight,
                                    bounds.width - 2.0f, bounds.height - headerHeight - 1.0f };
        const float pixelsPerUnit = content.height / camera.fovy;
        const float objectWidth = horizontalDimension * pixelsPerUnit;
        const float objectHeight = verticalDimension * pixelsPerUnit;
        const Vector2 center = { content.x + content.width * 0.5f,
                                 content.y + content.height * 0.5f };
        const float left = center.x - objectWidth * 0.5f;
        const float right = center.x + objectWidth * 0.5f;
        const float top = center.y - objectHeight * 0.5f;
        const float bottom = center.y + objectHeight * 0.5f;
        const float horizontalY = std::min(content.y + content.height - 14.0f, bottom + 13.0f);
        const float verticalX = std::min(content.x + content.width - 13.0f, right + 14.0f);
        const float arrowSize = 5.0f;

        // Horizontal dimension and extension lines.
        DrawLineV({ left, bottom + 2.0f }, { left, horizontalY + 5.0f }, dimensionColor);
        DrawLineV({ right, bottom + 2.0f }, { right, horizontalY + 5.0f }, dimensionColor);
        DrawLineEx({ left, horizontalY }, { right, horizontalY }, 2.0f, dimensionColor);
        DrawTriangle({ left, horizontalY }, { left + arrowSize, horizontalY - arrowSize },
            { left + arrowSize, horizontalY + arrowSize }, dimensionColor);
        DrawTriangle({ right, horizontalY }, { right - arrowSize, horizontalY + arrowSize },
            { right - arrowSize, horizontalY - arrowSize }, dimensionColor);
        const char* horizontalText = TextFormat("%.2f", horizontalDimension);
        const Vector2 horizontalTextSize = MeasureThemeText(horizontalText, 13.0f);
        DrawRectangle(static_cast<int>(center.x - horizontalTextSize.x * 0.5f - 3.0f),
            static_cast<int>(horizontalY - 8.0f),
            static_cast<int>(horizontalTextSize.x + 6.0f), 16,
            GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        DrawThemeText(horizontalText, center.x - horizontalTextSize.x * 0.5f,
            horizontalY - 7.0f, 13.0f, dimensionColor);

        // Vertical dimension and extension lines.
        DrawLineV({ right + 2.0f, top }, { verticalX + 5.0f, top }, dimensionColor);
        DrawLineV({ right + 2.0f, bottom }, { verticalX + 5.0f, bottom }, dimensionColor);
        DrawLineEx({ verticalX, top }, { verticalX, bottom }, 2.0f, dimensionColor);
        DrawTriangle({ verticalX, top }, { verticalX - arrowSize, top + arrowSize },
            { verticalX + arrowSize, top + arrowSize }, dimensionColor);
        DrawTriangle({ verticalX, bottom }, { verticalX + arrowSize, bottom - arrowSize },
            { verticalX - arrowSize, bottom - arrowSize }, dimensionColor);
        const char* verticalText = TextFormat("%.2f", verticalDimension);
        const Vector2 verticalTextSize = MeasureThemeText(verticalText, 13.0f);
        DrawRectangle(static_cast<int>(verticalX - verticalTextSize.x * 0.5f - 3.0f),
            static_cast<int>(center.y - 8.0f),
            static_cast<int>(verticalTextSize.x + 6.0f), 16,
            GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        DrawThemeText(verticalText, verticalX - verticalTextSize.x * 0.5f,
            center.y - 7.0f, 13.0f, dimensionColor);
    }

    static void DrawGuidedReferenceViews()
    {
        const Rectangle dock = getEditorDockBounds();
        const float gap = 4.0f;
        const float viewHeight = (dock.height - gap * 2.0f) / 3.0f;
        const float contentHeight = std::max(1.0f, viewHeight - 31.0f);
        const float aspect = std::max(0.1f, (dock.width - 2.0f) / contentHeight);
        const float padding = 1.18f;
        const GuidedObjectFrame frame = GetGuidedObjectFrame();
        const float cameraDistance = 5.0f + 2.0f * std::max(
            frame.halfExtent.x, std::max(frame.halfExtent.y, frame.halfExtent.z));

        const float frontHeight = 2.0f * padding *
            std::max(frame.halfExtent.y, frame.halfExtent.x / aspect);
        const float topHeight = 2.0f * padding *
            std::max(frame.halfExtent.z, frame.halfExtent.x / aspect);
        const float sideHeight = 2.0f * padding *
            std::max(frame.halfExtent.y, frame.halfExtent.z / aspect);
        const Camera3D front = MakeGuidedReferenceCamera(
            { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, frame.center,
            frontHeight, cameraDistance);
        const Camera3D top = MakeGuidedReferenceCamera(
            { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, frame.center,
            topHeight, cameraDistance);
        const Camera3D side = MakeGuidedReferenceCamera(
            { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, frame.center,
            sideHeight, cameraDistance);

        DrawGuidedReferenceView({ dock.x, dock.y, dock.width, viewHeight }, "Front", front,
            frame.halfExtent.x * 2.0f, frame.halfExtent.y * 2.0f);
        DrawGuidedReferenceView({ dock.x, dock.y + viewHeight + gap, dock.width, viewHeight },
            "Top", top, frame.halfExtent.x * 2.0f,
            frame.halfExtent.z * 2.0f);
        DrawGuidedReferenceView({ dock.x, dock.y + (viewHeight + gap) * 2.0f,
                                  dock.width, viewHeight }, "Side", side,
            frame.halfExtent.z * 2.0f, frame.halfExtent.y * 2.0f);

        const char* shortcut = guidedDimensionsVisible ? "M: Dimensions ON" : "M: Dimensions";
        const Vector2 shortcutSize = MeasureThemeText(shortcut, 12.0f);
        DrawThemeText(shortcut, dock.x + dock.width - shortcutSize.x - 9.0f,
            dock.y + 8.0f, 12.0f,
            guidedDimensionsVisible ? Color{ 255, 196, 64, 255 }
        : GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    }

    cameraController& getEditableCamera() {
        for (auto& slot : freeDrawState.activeViews)
            if (slot.editable) return slot.camera;
        return freeDrawState.activeViews[0].camera; // fallback, should be unreachable
    }

    static std::vector<Rectangle> computeViewportBounds(int count)
    {
        float w = static_cast<float>(GetScreenWidth());
        float h = static_cast<float>(GetScreenHeight());
        std::vector<Rectangle> result;

        if (count <= 1)
        {
            result.push_back({ 0, 0, w, h });
            return result;
        }

        float leftW = w / 2.0f;
        float rightW = w - leftW;
        int rightCount = count - 1;
        float rightH = h / static_cast<float>(rightCount);

        result.push_back({ 0, 0, leftW, h });
        for (int i = 0; i < rightCount; ++i)
            result.push_back({ leftW, rightH * i, rightW, rightH });

        return result;
    }

    int selectedIndex = -1;


    float getCameraPanelHeight()
    {
        using namespace UiStyle;
        if (!freeDrawState.cameraPanelOpen) return kPanelHeaderHeight;

        float dockHeight = getEditorDockBounds().height;
        int closedCount = (!freeDrawState.workspacePanelOpen ? 1 : 0) + (!freeDrawState.propertiesPanelOpen ? 1 : 0);
        float openWeightSum = kCameraWeight
            + (freeDrawState.workspacePanelOpen ? kWorkspaceWeight : 0.0f)
            + (freeDrawState.propertiesPanelOpen ? kPropertiesWeight : 0.0f);

        if (openWeightSum <= 0.0f) return kPanelHeaderHeight;

        float available = dockHeight - closedCount * kPanelHeaderHeight;
        return available * (kCameraWeight / openWeightSum);
    }

    float getWorkspacePanelHeight()
    {
        using namespace UiStyle;
        if (!freeDrawState.workspacePanelOpen) return kPanelHeaderHeight;

        float dockHeight = getEditorDockBounds().height;
        int closedCount = (!freeDrawState.cameraPanelOpen ? 1 : 0) + (!freeDrawState.propertiesPanelOpen ? 1 : 0);
        float openWeightSum = kWorkspaceWeight + (freeDrawState.cameraPanelOpen ? kCameraWeight : 0.0f) + (freeDrawState.propertiesPanelOpen ? kPropertiesWeight : 0.0f);
        float available = dockHeight - closedCount * kPanelHeaderHeight;

        if (openWeightSum <= 0.0f) return kPanelHeaderHeight;

        return available * (kWorkspaceWeight / openWeightSum);
    }

    Rectangle getCameraPanelBounds() {
        Rectangle dock = getEditorDockBounds();
        return { dock.x, dock.y, dock.width, getCameraPanelHeight() };
    }
    Rectangle getWorkspacePanelBounds() {
        Rectangle dock = getEditorDockBounds();
        return { dock.x, dock.y + getCameraPanelHeight(), dock.width, getWorkspacePanelHeight() };
    }
    Rectangle getPropertiesPanelBounds() {
        Rectangle dock = getEditorDockBounds();
        float height = freeDrawState.propertiesPanelOpen ? (dock.height - getCameraPanelHeight() - getWorkspacePanelHeight()) : UiStyle::kPanelHeaderHeight;
        return { dock.x, dock.y + getCameraPanelHeight() + getWorkspacePanelHeight(), dock.width, height };
    }


    enum PropertyFloatField
    {
        PROPERTY_POSITION_X = 0, PROPERTY_POSITION_Y, PROPERTY_POSITION_Z,
        PROPERTY_ROTATION_X, PROPERTY_ROTATION_Y, PROPERTY_ROTATION_Z,
        PROPERTY_SCALE_X, PROPERTY_SCALE_Y, PROPERTY_SCALE_Z,
        PROPERTY_LIGHT_TARGET_X, PROPERTY_LIGHT_TARGET_Y, PROPERTY_LIGHT_TARGET_Z,
        PROPERTY_LIGHT_INTENSITY, PROPERTY_LIGHT_RADIUS,
        PROPERTY_FLOAT_FIELD_COUNT
    };

    struct floatFieldState { char text[32] = {}; };

    static floatFieldState propertyFloatFields[PROPERTY_FLOAT_FIELD_COUNT];
    static int activeFloatField = -1;
    static Vector2 cameraPanelScroll = { 0.0f, 0.0f };
    static Vector2 workspacePanelScroll = { 0.0f, 0.0f };
    static Vector2 propertiesPanelScroll = { 0.0f, 0.0f };

    enum CameraPropertyField
    {
        CAMERA_PROPERTY_FOV = 0, CAMERA_PROPERTY_MOVE_SPEED, CAMERA_PROPERTY_SENSITIVITY,
        CAMERA_PROPERTY_NEAR, CAMERA_PROPERTY_FAR, CAMERA_PROPERTY_FIELD_COUNT
    };


    // bloc: variables
    static floatFieldState cameraPropertyFields[CAMERA_PROPERTY_FIELD_COUNT];
    static int activeCameraPropertyField = -1;

    static bool propertyColorEditMode[4] = { false, false, false, false };
    static bool propertyMaterialDropdownOpen = false;
    static bool viewportClickCandidate = false;
    static Vector2 viewportClickStart = { 0.0f, 0.0f };

    // New: light target picking
    static bool lightTargetPickMode = false;
    static Light* lightTargetPickLight = nullptr;


    static bool isPropertyEditorActive()
    {
        if (propertyMaterialDropdownOpen) return true;
        if (activeFloatField != -1) return true;
        if (activeCameraPropertyField != -1) return true;
        if (freeDrawState.viewDropdownOpen) return true;
        return false;
    }

    static bool tryParseFloat(const char* text, float& result)
    {
        if (text == nullptr || text[0] == '\0') return false;

        char* end = nullptr;
        float parsed = std::strtof(text, &end);

        if (end == text) return false;

        while (*end == ' ' || *end == '\t') end++;

        if (*end != '\0') return false;
        if (!std::isfinite(parsed)) return false;

        result = parsed;
        return true;
    }

    static void setFloatFieldText(floatFieldState& field, float value) {
        std::snprintf(field.text, sizeof(field.text), "%.3f", value);
    }

    static void resetPropertyEditorState()
    {
        activeFloatField = -1;
        for (auto& field : propertyFloatFields) field.text[0] = '\0';
        for (bool& editMode : propertyColorEditMode) editMode = false;
        propertyMaterialDropdownOpen = false;
    }

    static shape* propertyBoundObject = nullptr;
    static Light* propertyBoundLight = nullptr;

    void updatePropertyBinding(shape* object)
    {
        if (propertyBoundObject == object && propertyBoundLight == nullptr) return;
        propertyBoundObject = object;
        propertyBoundLight = nullptr;
        resetPropertyEditorState();
    }

    void updatePropertyBinding(Light* light)
    {
        if (propertyBoundLight == light && propertyBoundObject == nullptr) return;
        propertyBoundLight = light;
        propertyBoundObject = nullptr;
        resetPropertyEditorState();
    }

    static bool drawFloatPropertyField(Rectangle bounds, int fieldIndex, float& value, float minimum, float maximum)
    {
        if (!(fieldIndex >= 0 && fieldIndex < PROPERTY_FLOAT_FIELD_COUNT)) return 0;
        floatFieldState& field = propertyFloatFields[fieldIndex];

        if (activeFloatField != fieldIndex) setFloatFieldText(field, value);

        bool editing = (activeFloatField == fieldIndex);
        int result = GuiTextBox(bounds, field.text, static_cast<int>(sizeof(field.text)), editing);

        bool changed = false;
        if (editing)
        {
            float parsedValue = value;
            if (tryParseFloat(field.text, parsedValue))
            {
                parsedValue = Clamp(parsedValue, minimum, maximum);
                if (fabsf(parsedValue - value) > 0.0001f) { value = parsedValue; changed = true; }
            }
        }

        if (result != 0)
        {
            activeFloatField = editing ? -1 : fieldIndex;
            editing = (activeFloatField == fieldIndex);
        }

        if (editing && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), bounds))
        {
            float parsedValue = value;
            if (tryParseFloat(field.text, parsedValue)) { value = Clamp(parsedValue, minimum, maximum); changed = true; }
            activeFloatField = -1;
            setFloatFieldText(field, value);
        }

        return changed;
    }

    static bool drawCameraFloatField(Rectangle bounds, int fieldIndex, float& value, float minimum, float maximum)
    {
        if (!(fieldIndex >= 0 && fieldIndex < CAMERA_PROPERTY_FIELD_COUNT)) return false;
        floatFieldState& field = cameraPropertyFields[fieldIndex];

        if (activeCameraPropertyField != fieldIndex) setFloatFieldText(field, value);

        bool editing = (activeCameraPropertyField == fieldIndex);
        int result = GuiTextBox(bounds, field.text, static_cast<int>(sizeof(field.text)), editing);

        bool changed = false;
        if (editing)
        {
            float parsed = value;
            if (tryParseFloat(field.text, parsed))
            {
                parsed = Clamp(parsed, minimum, maximum);
                if (fabsf(parsed - value) > 0.0001f) { value = parsed; changed = true; }
            }
        }

        if (result != 0)
        {
            activeCameraPropertyField = editing ? -1 : fieldIndex;
            editing = (activeCameraPropertyField == fieldIndex);
        }

        if (editing && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), bounds))
        {
            float parsed = value;
            if (tryParseFloat(field.text, parsed)) { value = Clamp(parsed, minimum, maximum); changed = true; }
            activeCameraPropertyField = -1;
            setFloatFieldText(field, value);
        }

        return changed;
    }

    static void drawSectionDivider(float x, float& y, float width)
    {
        using namespace UiStyle;
        y += kDividerMargin;
        Color line = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), 1, Fade(line, 0.6f));
        y += kDividerMargin;
    }

    static void drawSectionLabel(float x, float& y, const char* label)
    {
        using namespace UiStyle;
        DrawRectangle(static_cast<int>(x), static_cast<int>(y + 3), 3, 10, kAccent);
        DrawTextEx(GuiGetFont(), label, { x + 8.0f, y }, fontSize, spacing,
            Fade(GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)), 0.85f));
        y += 19.0f;
    }

    static bool DrawVector3Property(
        const char* title, float x, float& y, float width, Vector3& value, int firstFieldIndex, float minimum, float maximum)
    {
        using namespace UiStyle;
        drawSectionLabel(x, y, title);

        bool changed = 0;

        constexpr float gap = 6.0f;
        constexpr float labelWidth = 14.0f;
        const float fieldHeight = editorControlHeight;
        const float groupWidth = (width - gap * 2.0f) / 3.0f;

        const char* labels[3] = { "X", "Y", "Z" };
        float* components[3] = { &value.x, &value.y, &value.z };

        for (int i = 0; i < 3; i++) {
            const float groupX = x + i * (groupWidth + gap);
            DrawTextEx(GuiGetFont(), labels[i], { groupX, y + 5.0f }, fontSize, spacing,
                Fade(GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)), 0.7f));

            Rectangle fieldBounds = { groupX + labelWidth, y, groupWidth - labelWidth, fieldHeight };

            bool isActive = (activeFloatField == firstFieldIndex + i);
            changed |= drawFloatPropertyField(fieldBounds, firstFieldIndex + i, *components[i], minimum, maximum);
            if (isActive) {
                DrawRectangleLinesEx({ fieldBounds.x - 1, fieldBounds.y - 1, fieldBounds.width + 2, fieldBounds.height + 2 }, 1.5f, kAccent);
            }
        }
        y += fieldHeight + kFieldGap;
        return changed;
    }

    bool isPointerOverEditorUi()
    {
        if (IsCursorHidden()) return false; // GetMousePosition() is unbounded/meaningless while hidden
        return CheckCollisionPointRec(GetMousePosition(), getEditorDockBounds());
    }

    static bool drawPanelFrame(Rectangle bounds, const char* title, bool open)
    {
        using namespace UiStyle;

        const float toggleWidth = 22.0f;
        const float collapsedWidth = 140.0f;

        Rectangle headerBounds = bounds;
        if (!open)
        {
            headerBounds.width = collapsedWidth;
            headerBounds.x = bounds.x + bounds.width - collapsedWidth;
        }

        const Color background = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
        const Color border = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
        const Color text = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
        const Color header = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));

        DrawRectangle(static_cast<int>(headerBounds.x + 3), static_cast<int>(headerBounds.y + 3), static_cast<int>(headerBounds.width), static_cast<int>(headerBounds.height), kShadow);
        DrawRectangleRec(headerBounds, Fade(background, 0.97f));
        DrawRectangleLinesEx(headerBounds, 1.0f, border);
        DrawRectangle(static_cast<int>(headerBounds.x), static_cast<int>(headerBounds.y), static_cast<int>(headerBounds.width), 32, header);
        DrawRectangle(static_cast<int>(headerBounds.x), static_cast<int>(headerBounds.y + 32), static_cast<int>(headerBounds.width), 2, kAccent);
        DrawTextEx(GuiGetFont(), title, { headerBounds.x + 10.0f, headerBounds.y + 7.0f }, fontSize, spacing, text);

        Rectangle toggleBounds = { headerBounds.x + headerBounds.width - toggleWidth, headerBounds.y, toggleWidth, 32.0f };
        bool pressed = GuiButton(toggleBounds, "");

        const char* toggleLabel = open ? "CLOSE" : "OPEN";
        Vector2 labelSize = MeasureTextEx(GuiGetFont(), toggleLabel, fontSize, spacing);
        Vector2 labelPos = {
            toggleBounds.x + toggleBounds.width / 2.0f + labelSize.x / 2.0f - 4.0f,
            toggleBounds.y + toggleBounds.height / 2.0f + labelSize.y / 2.0f
        };
        DrawTextPro(GuiGetFont(), toggleLabel, labelPos, { 0, 0 }, -90.0f, fontSize, spacing, text);

        return pressed;
    }

    static void drawWorkspacePanel(bool interactive)
    {
        if (!interactive) GuiDisable();

        Rectangle panel = getWorkspacePanelBounds();

        if (drawPanelFrame(panel, "Workspace", freeDrawState.workspacePanelOpen))
            freeDrawState.workspacePanelOpen = !freeDrawState.workspacePanelOpen;

        if (!freeDrawState.workspacePanelOpen)
        {
            if (!interactive) GuiEnable();
            return;
        }

        const float rowHeight = static_cast<float>(std::max(26, GuiGetStyle(DEFAULT, TEXT_SIZE) + 12));
        const float headerH = 39.0f;
        const float sectionLabelHeight = 19.0f;

        Rectangle scrollBounds = { panel.x, panel.y + headerH, panel.width, panel.height - headerH };

        int objectCount = static_cast<int>(objects.size());
        int lightCount = static_cast<int>(lights.size());

        float contentHeight = std::max(
            scrollBounds.height,
            sectionLabelHeight + objectCount * (rowHeight + 1.0f)
            + sectionLabelHeight + lightCount * (rowHeight + 1.0f)
            + 10.0f
        );

        Rectangle content = { 0, 0, scrollBounds.width - 16.0f, contentHeight };
        Rectangle panelView;
        GuiScrollPanel(scrollBounds, NULL, content, &workspacePanelScroll, &panelView);

        BeginScissorMode(panelView.x, panelView.y, panelView.width, panelView.height);

        float y = scrollBounds.y + workspacePanelScroll.y + 4.0f;

        drawSectionLabel(panel.x + 20.0f, y, "Objects");

        if (objectCount == 0)
        {
            DrawTextEx(GuiGetFont(), "Workspace is empty", { panel.x + 20.0f, y + 3.0f }, fontSize, spacing,
                GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
            y += rowHeight;
        }
        else
        {
            int rowIndex = 0;
            for (auto& objectPtr : objects)
            {
                shape* object = objectPtr.get();
                if (object == nullptr) continue;

                Rectangle row = { panel.x + 20.0f, y, scrollBounds.width - 36.0f, rowHeight };

                if ((rowIndex % 2) == 1)
                    DrawRectangleRec(row, Fade(GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)), 0.12f));

                const char* label = TextFormat("(%d) %s", object->getId(), object->getObjectTypeString());
                const bool wasSelected = object->getSelected();
                bool rowSelected = wasSelected;

                const int previousAlignment = GuiGetStyle(TOGGLE, TEXT_ALIGNMENT);
                GuiSetStyle(TOGGLE, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                GuiToggle(row, label, &rowSelected);
                GuiSetStyle(TOGGLE, TEXT_ALIGNMENT, previousAlignment);

                if (rowSelected)
                    DrawRectangle(static_cast<int>(row.x), static_cast<int>(row.y), 3, static_cast<int>(row.height), UiStyle::kAccent);

                if (rowSelected != wasSelected)
                {
                    const bool additive = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
                    selectObjects(object, additive);
                }

                y += rowHeight + 1.0f;
                ++rowIndex;
            }
        }

        y += UiStyle::kDividerMargin;
        drawSectionLabel(panel.x + 20.0f, y, "Lights");

        if (lightCount == 0)
        {
            DrawTextEx(GuiGetFont(), "No lights in scene", { panel.x + 20.0f, y + 3.0f }, fontSize, spacing,
                GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
        }
        else
        {
            int rowIndex = 0;
            for (auto& lightPtr : lights)
            {
                Light* lightObj = lightPtr.get();
                if (lightObj == nullptr) continue;

                Rectangle row = { panel.x + 20.0f, y, scrollBounds.width - 36.0f, rowHeight };

                if ((rowIndex % 2) == 1)
                    DrawRectangleRec(row, Fade(GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)), 0.12f));

                const char* label = TextFormat("Light %d", lightObj->getId());
                const bool wasSelected = lightObj->getSelected();
                bool rowSelected = wasSelected;

                const int previousAlignment = GuiGetStyle(TOGGLE, TEXT_ALIGNMENT);
                GuiSetStyle(TOGGLE, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                GuiToggle(row, label, &rowSelected);
                GuiSetStyle(TOGGLE, TEXT_ALIGNMENT, previousAlignment);

                if (rowSelected)
                    DrawRectangle(static_cast<int>(row.x), static_cast<int>(row.y), 3, static_cast<int>(row.height), UiStyle::kAccent);

                if (rowSelected != wasSelected)
                {
                    const bool additive = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
                    selectLights(lightObj, additive);
                }

                y += rowHeight + 1.0f;
                ++rowIndex;
            }
        }

        EndScissorMode();

        if (!interactive) GuiEnable();
    }

    static void setCameraNavigationMode(cameraController& cam, cameraNavigationMode mode)
    {
        if (cam.getNavigationMode() == mode) return;

        cam.setNavigationMode(mode);

        if (mode == cameraNavigationMode::Walk)
            DisableCursor();   // hides + locks the cursor for FPS-style mouselook
        else
            EnableCursor();    // restores the cursor for orbit/UI interaction
    }

    static void drawCameraPanel(bool interactive)
    {
        if (!(interactive || freeDrawState.viewDropdownOpen)) GuiDisable();

        Rectangle panel = getCameraPanelBounds();
        if (drawPanelFrame(panel, "Camera", freeDrawState.cameraPanelOpen))
            freeDrawState.cameraPanelOpen = !freeDrawState.cameraPanelOpen;

        if (!freeDrawState.cameraPanelOpen)
        {
            if (!interactive) GuiEnable();
            return;
        }

        const float contentX = panel.x + 10.0f;
        const float contentWidth = panel.width - 20.0f;
        const float controlHeight = editorControlHeight;

        float pinnedY = panel.y + 39.0f;

        DrawTextEx(GuiGetFont(), TextFormat("Projection: %s", getEditableCamera().getCameraProjection()),
            { contentX, pinnedY + 5.0f }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
        pinnedY += 24.0f;

        DrawTextEx(GuiGetFont(), "View", { contentX, pinnedY + 5.0f }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

        Rectangle viewRect = { contentX + 45.0f, pinnedY, contentWidth - 45.0f, controlHeight };
        const char* viewOptions = "Free;Front;Top;Left;Right";
        static int activeViewIndex = static_cast<int>(freeDrawState.currentViewIndex);

        if (!freeDrawState.viewDropdownOpen && activeViewIndex != static_cast<int>(freeDrawState.currentViewIndex))
            activeViewIndex = static_cast<int>(freeDrawState.currentViewIndex);

        if (GuiDropdownBox(viewRect, viewOptions, &activeViewIndex, freeDrawState.viewDropdownOpen))
            freeDrawState.viewDropdownOpen = !freeDrawState.viewDropdownOpen;

        if (freeDrawState.viewDropdownOpen && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            !CheckCollisionPointRec(GetMousePosition(), viewRect))
        {
            Rectangle listArea = { viewRect.x, viewRect.y + viewRect.height, viewRect.width, controlHeight * 5.0f };
            if (!CheckCollisionPointRec(GetMousePosition(), listArea))
                freeDrawState.viewDropdownOpen = false;
        }

        freeDrawState.currentViewIndex = static_cast<cameraView>(activeViewIndex);

        if (freeDrawState.currentViewIndex != freeDrawState.lastViewIndex)
        {
            Vector3 focus = activeObject ? activeObject->getTransform().translation : Vector3{ 0.0f, 0.0f, 0.0f };
            getEditableCamera().setView(freeDrawState.currentViewIndex, focus);
            freeDrawState.lastViewIndex = freeDrawState.currentViewIndex;
            freeDrawState.cameraLocked = (freeDrawState.currentViewIndex != cameraView::Free);
        }

        pinnedY += controlHeight + 12.0f;

        if (freeDrawState.viewDropdownOpen)
        {
            if (!interactive) GuiEnable();
            return;
        }

        Rectangle scrollBounds = { panel.x, pinnedY, panel.width, panel.y + panel.height - pinnedY };
        if (scrollBounds.height < 20.0f) scrollBounds.height = 20.0f;

        const float fieldBlockHeight = 44.0f;
        const float navSectionHeight = 19.0f + UiStyle::kDividerMargin * 2.0f
            + (controlHeight + 8.0f) + (controlHeight + 12.0f);
        const float contentHeight = 20.0f + fieldBlockHeight * CAMERA_PROPERTY_FIELD_COUNT
            + 30.0f + controlHeight + 20.0f + navSectionHeight;

        Rectangle content = { 0, 0, scrollBounds.width - 16.0f, contentHeight };
        Rectangle panelView;
        GuiScrollPanel(scrollBounds, NULL, content, &cameraPanelScroll, &panelView);

        BeginScissorMode(panelView.x, panelView.y, panelView.width, panelView.height);

        float y = scrollBounds.y + cameraPanelScroll.y + 8.0f;
        float fx = contentX;
        float fw = contentWidth - 16.0f;

        drawSectionLabel(fx, y, "Camera Settings");

        struct { const char* label; float* value; float minimum; float maximum; int index; }
        camFields[] = {
            { "FOV", &getEditableCamera().getFovy(), 5.0f, 120.0f, CAMERA_PROPERTY_FOV },
            { "Move Speed",  &getEditableCamera().getWalkSpeed(),        0.01f,  1000.0f,    CAMERA_PROPERTY_MOVE_SPEED},
            { "Sensitivity", &getEditableCamera().getMouseSensitivity(), 0.001f,   10.0f,    CAMERA_PROPERTY_SENSITIVITY},
        };
        bool result = false;
        for (auto& f : camFields)
        {
            DrawTextEx(GuiGetFont(), f.label, { fx, y + 4.0f }, fontSize, spacing, Fade(GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)), 0.85f));
            y += 17.0f;
            Rectangle fieldBounds = { fx, y, fw, controlHeight };
            result |= drawCameraFloatField(fieldBounds, f.index, *f.value, f.minimum, f.maximum);
            y += controlHeight + 12.0f;
        }
        if (result) getEditableCamera().syncCamera();

        y += 8.0f;
        drawSectionDivider(fx, y, fw);
        drawSectionLabel(fx, y, "Display");

        Rectangle splitBtn = { fx, y, fw, controlHeight };
        bool splitActive = freeDrawState.splitScreenEnabled;
        GuiToggle(splitBtn, splitActive ? "Split Screen: On" : "Split Screen: Off", &splitActive);

        if (splitActive && freeDrawState.activeViews.size() == 1)
        {
            freeDrawState.splitScreenEnabled = splitActive;
            ViewportSlot front, top, left;
            front.editable = false; front.trackSelection = true; front.presetView = cameraView::Front;
            top.editable = false; top.trackSelection = true; top.presetView = cameraView::Top;
            left.editable = false; left.trackSelection = true; left.presetView = cameraView::Left;

            Vector3 focus = activeObject ? activeObject->getTransform().translation : Vector3{ 0,0,0 };

            front.camera.setView(cameraView::Front, focus);
            top.camera.setView(cameraView::Top, focus);
            left.camera.setView(cameraView::Left, focus);

            freeDrawState.activeViews.push_back(front);
            freeDrawState.activeViews.push_back(top);
            freeDrawState.activeViews.push_back(left);
        }
        else if (!splitActive && freeDrawState.activeViews.size() > 1)
        {
            // Release each discarded slot's render target before dropping it —
            // vector::resize won't do this for us since RenderTexture2D has no destructor.
            for (size_t i = 1; i < freeDrawState.activeViews.size(); ++i)
                freeDrawState.activeViews[i].releaseTarget();

            freeDrawState.activeViews.resize(1);
        }

        freeDrawState.splitScreenEnabled = splitActive;

        y += controlHeight + 12.0f;

        drawSectionDivider(fx, y, fw);
        drawSectionLabel(fx, y, "Navigation");

        Rectangle navModeBtn = { fx, y, fw, controlHeight };
        bool orbitMode = (getEditableCamera().getNavigationMode() == cameraNavigationMode::Orbit);
        bool orbitModeToggled = orbitMode;
        GuiToggle(navModeBtn, orbitMode ? "Mode: Orbit" : "Mode: Walk", & orbitModeToggled);
        if (orbitModeToggled != orbitMode)
        {
            setCameraNavigationMode(getEditableCamera(), orbitModeToggled ? cameraNavigationMode::Orbit : cameraNavigationMode::Walk);
        }
        y += controlHeight + 8.0f;

        Rectangle projectionBtn = { fx, y, fw, controlHeight };
        if (GuiButton(projectionBtn, TextFormat("Projection: %s (click to toggle)", getEditableCamera().getCameraProjection())))
        {
            getEditableCamera().toggleProjection();
        }
        y += controlHeight + 12.0f;

        EndScissorMode();

        if (!interactive) GuiEnable();
    }
    void drawPropertiesPanel(bool interactive)
    {
        if (!interactive) GuiDisable();
        const int totalObjects = static_cast<int>(selectedObjects.size());
        const int totalLights = static_cast<int>(selectedLights.size());
        const int total = totalObjects + totalLights;

        Rectangle panel = getPropertiesPanelBounds();
        if (drawPanelFrame(panel, "Properties", freeDrawState.propertiesPanelOpen))
            freeDrawState.propertiesPanelOpen = !freeDrawState.propertiesPanelOpen;

        if (!freeDrawState.propertiesPanelOpen)
        {
            if (!interactive) GuiEnable();
            return;
        }

        const float contentX = panel.x + 10.0f;
        const float contentWidth = panel.width - 20.0f;
        float y = panel.y + 39.0f;

        if (total == 0)
        {
            propertyBoundObject = nullptr;
            propertyBoundLight = nullptr;
            resetPropertyEditorState();
            DrawTextEx(GuiGetFont(), "No object selected", { contentX, y }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
            if (!interactive) GuiEnable();
            return;
        }

        if (total > 1)
        {
            propertyBoundObject = nullptr;
            propertyBoundLight = nullptr;
            resetPropertyEditorState();
            DrawTextEx(GuiGetFont(), TextFormat("Multiple selected: %d", total), { contentX, y }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
            if (!interactive) GuiEnable();
            return;
        }

        // Exactly one thing selected — either a light or a shape.
        if (totalLights == 1)
        {
            Light* selected = activeLight ? activeLight : selectedLights.front();
            updatePropertyBinding(selected);

            DrawTextEx(GuiGetFont(), TextFormat("Light %d", selected->getId()), { contentX, y }, fontSize + 2.0f, spacing, UiStyle::kAccent);
            y += 25.0f;

            Rectangle scrollBounds = { panel.x, y, panel.width, panel.y + panel.height - y - (editorControlHeight + 20.0f) };

            const float vec3BlockHeight = 19.0f + editorControlHeight + 10.0f + UiStyle::kDividerMargin * 2.0f + 1.0f;
            const float pickButtonBlockHeight = editorControlHeight + UiStyle::kFieldGap;
            const float contentHeight = vec3BlockHeight * 2.0f + pickButtonBlockHeight + 20.0f;

            Rectangle content = { 0, 0, scrollBounds.width - 16.0f, contentHeight };
            Rectangle view;
            GuiScrollPanel(scrollBounds, NULL, content, &propertiesPanelScroll, &view);

            BeginScissorMode(view.x, view.y, view.width, view.height);

            float sy = scrollBounds.y + propertiesPanelScroll.y + 6.0f;
            float sw = contentWidth - 16.0f;

            drawSectionDivider(contentX, sy, sw);
            Vector3 position = selected->getPosition();
            bool positionChanged = DrawVector3Property("Position", contentX, sy, sw, position, PROPERTY_POSITION_X, -100000.0f, 100000.0f);
            if (positionChanged) selected->setPosition(position);

            drawSectionDivider(contentX, sy, sw);
            Vector3 target = selected->getTarget();
            bool targetChanged = DrawVector3Property("Look At", contentX, sy, sw, target, PROPERTY_LIGHT_TARGET_X, -100000.0f, 100000.0f);
            if (targetChanged) selected->setTarget(target);

            const bool pickingThisLight = (lightTargetPickMode && lightTargetPickLight == selected);
            Rectangle pickButtonBounds = { contentX, sy, sw, editorControlHeight };
            const char* pickLabel = pickingThisLight ? "Click in viewport to set target..." : "Pick Target in Viewport";
            if (GuiButton(pickButtonBounds, pickLabel) && !pickingThisLight)
            {
                lightTargetPickMode = true;
                lightTargetPickLight = selected;
            }
            sy += editorControlHeight + UiStyle::kFieldGap;

            EndScissorMode();

            Rectangle deselectButton = { contentX, panel.y + panel.height - editorControlHeight - 10.0f, 110.0f, editorControlHeight };
            if (GuiButton(deselectButton, "Deselect"))
            {
                selectedLights.clear();
                for (auto& lightPtr : lights) lightPtr->setSelected(false);
                activeLight = nullptr;
                resetPropertyEditorState();
            }

            if (!interactive) GuiEnable();
            return;
        }

        // --- totalObjects == 1: existing shape-property path, unchanged below ---
        shape* selected = activeObject;
        if (selected == nullptr) { if (!interactive) GuiEnable(); return; }

        updatePropertyBinding(selected);

        DrawTextEx(GuiGetFont(), TextFormat("%s %d", selected->getObjectTypeString(), selected->getId()), { contentX, y }, fontSize + 2.0f, spacing, UiStyle::kAccent);
        y += 25.0f;

        DrawTextEx(GuiGetFont(), "Material", { contentX, y + 5.0f }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

        int materialIndex = selected->getMaterialType();
        const float controlHeight = editorControlHeight;
        Rectangle materialBounds = { contentX + 82.0f, y, contentWidth - 82.0f, controlHeight };

        if (GuiDropdownBox(materialBounds, "Concrete;Wood;Plastic;Cobblestone;Brick;Tiles;Metal;Marble;Asphalt",
            &materialIndex, propertyMaterialDropdownOpen))
        {
            propertyMaterialDropdownOpen = !propertyMaterialDropdownOpen;
        }
        if (materialIndex != selected->getMaterialType())
            selected->applyMaterial(static_cast<MaterialType>(materialIndex));

        y += controlHeight + 10.0f;

        if (propertyMaterialDropdownOpen)
        {
            if (!interactive) GuiEnable();
            return;
        }

        Rectangle scrollBounds = { panel.x, y, panel.width, panel.y + panel.height - y - (controlHeight + 20.0f) };

        const float vec3BlockHeight = 19.0f + editorControlHeight + 10.0f + UiStyle::kDividerMargin * 2.0f + 1.0f;
        const float contentHeight = vec3BlockHeight * 3.0f + 20.0f;

        Rectangle content = { 0, 0, scrollBounds.width - 16.0f, contentHeight };
        Rectangle view;
        GuiScrollPanel(scrollBounds, NULL, content, &propertiesPanelScroll, &view);

        BeginScissorMode(view.x, view.y, view.width, view.height);

        float sy = scrollBounds.y + propertiesPanelScroll.y + 6.0f;
        float sw = contentWidth - 16.0f;

        drawSectionDivider(contentX, sy, sw);
        DrawVector3Property("Position", contentX, sy, sw, selected->getTransform().translation, PROPERTY_POSITION_X, -100000.0f, 100000.0f);

        drawSectionDivider(contentX, sy, sw);
        Vector3 eulerRotation = QuaternionToEuler(selected->getTransform().rotation);
        Vector3 eulerRotationDeg = Vector3Scale(eulerRotation, RAD2DEG);
        bool rotationChanged = DrawVector3Property("Rotation", contentX, sy, sw, eulerRotationDeg, PROPERTY_ROTATION_X, -360000.0f, 360000.0f);
        if (rotationChanged)
        {
            eulerRotation = Vector3Scale(eulerRotationDeg, DEG2RAD);
            Transform t = { selected->getTransform().translation,
                             QuaternionFromEuler(eulerRotation.x, eulerRotation.y, eulerRotation.z),
                             selected->getTransform().scale };
            selected->setTransform(t);
        }

        drawSectionDivider(contentX, sy, sw);
        DrawVector3Property("Scale", contentX, sy, sw, selected->getTransform().scale, PROPERTY_SCALE_X, 0.01f, 10000.0f);

        EndScissorMode();

        Rectangle deselectButton = { contentX, panel.y + panel.height - editorControlHeight - 10.0f, 110.0f, editorControlHeight };
        if (GuiButton(deselectButton, "Deselect"))
        {
            selectedObjects.clear();
            for (auto& object : objects) object->setSelected(false);
            resetPropertyEditorState();
        }

        if (!interactive) GuiEnable();
    }
    static void drawControlHintsOverlay()
    {
        const bool isWalkMode = (getEditableCamera().getNavigationMode() == cameraNavigationMode::Walk);

        struct HintLine { const char* text; };
        HintLine lines[] = {
			{ isWalkMode ? "Disable Walk Mode: N" : "" },
            { "Toggle Help: F1" },
        };
        const int lineCount = static_cast<int>(sizeof(lines) / sizeof(lines[0]));

        const float padding = 8.0f;
        const float lineHeight = fontSize + 5.0f;
        const float hintFontSize = fontSize;

        float maxWidth = 0.0f;
        for (auto& line : lines)
        {
            Vector2 size = MeasureTextEx(GuiGetFont(), line.text, hintFontSize, spacing);
            maxWidth = std::max(maxWidth, size.x);
        }

        const float boxX = 10.0f;
        const float boxY = 52.0f; // sits just below the top bar, matching getEditorDockBounds()'s y origin
        const float boxWidth = maxWidth + padding * 2.0f;
        const float boxHeight = lineCount * lineHeight + padding * 2.0f;

        DrawRectangle(static_cast<int>(boxX), static_cast<int>(boxY), static_cast<int>(boxWidth), static_cast<int>(boxHeight), Fade(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)), 0.75f));
        DrawRectangleLinesEx({ boxX, boxY, boxWidth, boxHeight }, 1.0f, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

        float ty = boxY + padding;
        for (auto& line : lines)
        {
            const Color color = isWalkMode && line.text[0] == 'D'
                ? UiStyle::kAccent // highlight the walk-mode line's color when it's the "disable" state
                : GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));

            DrawTextEx(GuiGetFont(), line.text, { boxX + padding, ty }, hintFontSize, spacing, color);
            ty += lineHeight;
        }
    }
}

void freeDrawInit() {
    TraceLog(LOG_INFO, "Initializing Free Draw Mode Scene");
    TraceLog(LOG_INFO, "%d", static_cast<int>(currentScene));

    if (freeDrawState.initiliased) return;
    InitTransformGizmo();
    freeDrawState.drawArea = { 200,140,220,44 };
    freeDrawState.initiliased = true;
    freeDrawState.mouseButtonPressed = false;
    freeDrawState.currentViewIndex = cameraView::Free;
    freeDrawState.lastViewIndex = cameraView::Free;
    freeDrawState.viewDropdownOpen = false;
    freeDrawState.cameraLocked = false;
    freeDrawState.helpTip = false;
    freeDrawState.splitScreenEnabled = false;
    freeDrawState.activeViews.clear();
    ViewportSlot mainSlot;
    mainSlot.editable = true;
    mainSlot.camera.cameraLookAt({ 10,10,10 }, { 0,0,0 }, { 0,1,0 }, CAMERA_PERSPECTIVE);
    freeDrawState.activeViews.push_back(mainSlot);

    initialiseEnvironment();
}

void freeDrawUpdate() {

    if (!freeDrawState.initiliased) return;
    if (guidedWorkspace && IsKeyPressed(KEY_M))
        guidedDimensionsVisible = !guidedDimensionsVisible;

    std::vector<Rectangle> bounds = computeViewportBounds(static_cast<int>(freeDrawState.activeViews.size()));

    if (lightTargetPickMode)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            lightTargetPickMode = false;
            lightTargetPickLight = nullptr;
        }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isPointerOverEditorUi())
        {
            Vector2 mouse = GetMousePosition();
            int hitIndex = -1;
            for (size_t i = 0; i < bounds.size(); ++i)
            {
                if (CheckCollisionPointRec(mouse, bounds[i])) { hitIndex = static_cast<int>(i); break; }
            }

            if (hitIndex != -1 && lightTargetPickLight != nullptr)
            {
                Rectangle vb = bounds[hitIndex];
                Vector2 localMouse = { mouse.x - vb.x, mouse.y - vb.y };
                Ray ray = GetScreenToWorldRayEx(
                    localMouse,
                    freeDrawState.activeViews[hitIndex].camera.getCamera(),
                    static_cast<int>(vb.width),
                    static_cast<int>(vb.height)
                );

                Vector3 hitPoint = {};
                bool hasHit = false;
                float closestDist = FLT_MAX;

                // Prefer hitting an actual object in the scene.
                for (auto& objectPtr : objects)
                {
                    RayCollision collision = GetRayCollisionBox(ray, objectPtr->getWorldBoundingBox());
                    if (collision.hit && collision.distance < closestDist)
                    {
                        closestDist = collision.distance;
                        hitPoint = collision.point;
                        hasHit = true;
                    }
                }

                if (!hasHit)
                {
                    // Clicked empty space — fall back to a horizontal plane at the
                    // light's current target height so the pick still lands somewhere sensible.
                    float planeHeight = lightTargetPickLight->getTarget().y;
                    if (fabsf(ray.direction.y) > 0.0001f)
                    {
                        float t = (planeHeight - ray.position.y) / ray.direction.y;
                        if (t > 0.0f)
                        {
                            hitPoint = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
                            hasHit = true;
                        }
                    }
                }

                if (hasHit) lightTargetPickLight->setTarget(hitPoint);
            }

            lightTargetPickMode = false;
            lightTargetPickLight = nullptr;
        }

        return; // Swallow this frame entirely — no gizmo, selection, or camera orbit while picking.
    }
    if (!isPropertyEditorActive() && !isPointerOverEditorUi())
    {
        cameraController& editCam = getEditableCamera();

        if (IsKeyPressed(KEY_KP_5))
        {
            editCam.toggleProjection();
        }

        if (IsKeyPressed(KEY_N))
        {
            cameraNavigationMode next = (editCam.getNavigationMode() == cameraNavigationMode::Orbit)
                ? cameraNavigationMode::Walk : cameraNavigationMode::Orbit;
            setCameraNavigationMode(editCam, next);
            TraceLog(LOG_INFO, "Cursor hidden after mode switch: %s", IsCursorHidden() ? "yes" : "no");

        }
    }
    // --- existing gizmo/selection/camera code continues unchanged below ---
    int editableIndex = 0;

    for (size_t i = 0; i < freeDrawState.activeViews.size(); ++i)
    {
        if (freeDrawState.activeViews[i].editable) { editableIndex = static_cast<int>(i); break; }
    }

    bool usingGizmo = IsCursorHidden() ? false : updateObjectTransformGizmo( freeDrawState.activeViews[editableIndex].camera.getCamera(), bounds[editableIndex] );

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        viewportClickStart = GetMousePosition();
        viewportClickCandidate = !usingGizmo && !isPointerOverEditorUi() && !freeDrawState.mouseButtonPressed;
    }

    if (viewportClickCandidate && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && Vector2Distance(viewportClickStart, GetMousePosition()) > 4.0f)
    {
        viewportClickCandidate = false;
    }



    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (viewportClickCandidate && !IsCursorHidden() && !isPointerOverEditorUi() && !freeDrawState.mouseButtonPressed)
        {
            Vector2 mouse = GetMousePosition();
            int hitIndex = -1;
            for (size_t i = 0; i < bounds.size(); ++i)
            {
                if (CheckCollisionPointRec(mouse, bounds[i])) { hitIndex = static_cast<int>(i); break; }
            }

            if (hitIndex != -1)
            {
                Rectangle vb = bounds[hitIndex];
                Vector2 localMouse = { mouse.x - vb.x, mouse.y - vb.y };
                Ray ray = GetScreenToWorldRayEx(
                    localMouse,
                    freeDrawState.activeViews[hitIndex].camera.getCamera(),
                    static_cast<int>(vb.width),
                    static_cast<int>(vb.height)
                );

                selectAnyByRay(ray);
            }
        }
        viewportClickCandidate = false;
    }

    if (IsKeyPressed(KEY_F1))
    {
        freeDrawState.helpTip = !freeDrawState.helpTip;
    }

    if (!guidedWorkspace && IsKeyPressed(KEY_DELETE) && !isPropertyEditorActive())
    {
        deleteObjects();
        resetPropertyEditorState();
    }

    Vector3 focus = activeObject ? activeObject->getTransform().translation : Vector3{ 0,0,0 };
    for (auto& slot : freeDrawState.activeViews)
        if (!slot.editable && slot.trackSelection)
            slot.camera.setView(slot.presetView, focus);

    if (!usingGizmo && !isPointerOverEditorUi())
    {
        Vector2 mouse = GetMousePosition();
        for (size_t i = 0; i < freeDrawState.activeViews.size(); ++i)
        {
            ViewportSlot& slot = freeDrawState.activeViews[i];
            bool overThisViewport = IsCursorHidden() ? slot.editable : CheckCollisionPointRec(mouse, bounds[i]);
            if (slot.editable && !freeDrawState.cameraLocked && overThisViewport)
                slot.camera.updateCamera();
        }
    }
}

void freeDrawDraw() {

    std::vector<Rectangle> viewBounds = computeViewportBounds(static_cast<int>(freeDrawState.activeViews.size()));

    for (size_t i = 0; i < freeDrawState.activeViews.size(); ++i)
    {
        ViewportSlot& slot = freeDrawState.activeViews[i];
        Rectangle vb = viewBounds[i];

        // Cheap no-op most frames — reallocates only when this slot's pixel
        // size actually changed (window resize, split-screen toggle).
        slot.ensureTarget(static_cast<int>(vb.width), static_cast<int>(vb.height));

        DrawCameraScene(slot.camera.getCamera(), vb, slot.target);
    }

    const float iconSize = 32.0f;
    Rectangle btnOptionsIcon = { (float)GetScreenWidth() - iconSize - 10.0f, 10.0f, iconSize, iconSize };
    if (GuiButton(btnOptionsIcon, "")) {
        sceneManagerChangeScene(sceneId::SCENE_OPTIONS);
    }
    GuiDrawIcon(ICON_GEAR_BIG, btnOptionsIcon.x, btnOptionsIcon.y, 2, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

    if (!guidedWorkspace && (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !isPointerOverEditorUi()) || freeDrawState.mouseButtonPressed) {
        contextMenu(freeDrawState.mouseButtonPressed, getEditableCamera().getCamera());
    }

    if (guidedWorkspace)
    {
        // Fixed, display-only orthographic views replace all editable dock UI.
        DrawGuidedReferenceViews();
    }
    else if(!IsCursorHidden())
    {
        drawPropertiesPanel(CheckCollisionPointRec(GetMousePosition(), getPropertiesPanelBounds()));
        drawWorkspacePanel(CheckCollisionPointRec(GetMousePosition(), getWorkspacePanelBounds()));
        drawCameraPanel(CheckCollisionPointRec(GetMousePosition(), getCameraPanelBounds()));
        drawControlHintsOverlay();
    }
    if (freeDrawState.helpTip)
    {
        drawCameraControllerSettings();
    }
}

void freeDrawUnload() {
    freeDrawState.initiliased = false;
    UnloadTransformGizmo();
    if (guidedViewTexture.id != 0) { UnloadRenderTexture(guidedViewTexture); guidedViewTexture = {}; guidedViewTextureWidth = guidedViewTextureHeight = 0; }

    for (auto& slot : freeDrawState.activeViews)
        slot.releaseTarget();

    EnableCursor(); // don't leak a disabled/locked cursor state into whatever scene loads next

    // Models, material textures, and lighting are application-wide resources
    // initialized once by sceneManagerInit(). Destroying them here made a
    // second visit to Learn use invalid GPU resources and crash.
}