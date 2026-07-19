#include "FreeDrawMode.h"
#include "raylib.h"
#include "raygui.h"
#include "raymath.h"
#include <vector>
#include <memory>

namespace UiStyle
{
    constexpr float kPanelPadding = 12.0f;
    constexpr float kSectionGap = 16.0f;   // space between distinct groups (Material / Position / Rotation...)
    constexpr float kLabelToField = 6.0f;    // space between a label and the control under it
    constexpr float kFieldGap = 10.0f;   // space after a finished field group
    constexpr float kDividerMargin = 8.0f;    // space above/below a divider line
    constexpr float kPanelHeaderHeight = 34.0f; // header bar (32) + accent line (2) — a collapsed panel is just this
    constexpr float kCameraWeight = 0.25f;
    constexpr float kWorkspaceWeight = 0.20f;
    constexpr float kPropertiesWeight = 0.55f;

    const Color kAccent = { 90, 160, 255, 255 };   // focus / selection accent
    const Color kAccentSoft = { 90, 160, 255, 40 };    // faint accent fill (hover/alt rows)
    const Color kShadow = { 0, 0, 0, 35 };
}

namespace
{
    cameraController& getEditableCamera() {
        return freeDrawState.activeViews[0].camera;

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

    static Rectangle getEditorDockBounds()
    {
        // Keep vector values readable without consuming too much viewport at
        // lower resolutions.
        const float width = Clamp(GetScreenWidth() * 0.30f, 370.0f, 430.0f);
        return { static_cast<float>(GetScreenWidth()) - width, 52.0f, width, static_cast<float>(GetScreenHeight()) - 52.0f };
    }

    float getCameraPanelHeight()
    {
        using namespace UiStyle;
        if (!freeDrawState.cameraPanelOpen) return kPanelHeaderHeight;

        float dockHeight = getEditorDockBounds().height;
        int closedCount = (!freeDrawState.workspacePanelOpen ? 1 : 0) + (!freeDrawState.propertiesPanelOpen ? 1 : 0);
        float openWeightSum = kCameraWeight
            + (freeDrawState.workspacePanelOpen ? kWorkspaceWeight : 0.0f)
            + (freeDrawState.propertiesPanelOpen ? kPropertiesWeight : 0.0f);

        if (openWeightSum <= 0.0f) return kPanelHeaderHeight; // guard divide-by-zero when all panels are closed

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

        if (openWeightSum <= 0.0f) return kPanelHeaderHeight; // guard divide-by-zero when all panels are closed

        return available * (kWorkspaceWeight / openWeightSum);
    }

    Rectangle getCameraPanelBounds() {
        Rectangle dock = getEditorDockBounds();

        Rectangle panel = {
            dock.x,
            dock.y,
            dock.width,
            getCameraPanelHeight()
        };

        return panel;
    }
    Rectangle getWorkspacePanelBounds() {
        Rectangle dock = getEditorDockBounds();
        Rectangle panel = {
            dock.x,
            dock.y + getCameraPanelHeight(),
            dock.width,
            getWorkspacePanelHeight()
        };
        return panel;
    }
    Rectangle getPropertiesPanelBounds() {
        Rectangle dock = getEditorDockBounds();

        float height = freeDrawState.propertiesPanelOpen ? (dock.height - getCameraPanelHeight() - getWorkspacePanelHeight()) : UiStyle::kPanelHeaderHeight;

        Rectangle panel = { dock.x, dock.y + getCameraPanelHeight() + getWorkspacePanelHeight(), dock.width, height };
        return panel;
    }

    const float fontSize = static_cast<float>(GuiGetStyle(DEFAULT, TEXT_SIZE));
    const float spacing = static_cast<float>(std::max(0, GuiGetStyle(DEFAULT, TEXT_SPACING)));
    const float editorControlHeight = std::max(27, GuiGetStyle(DEFAULT, TEXT_SIZE) + 11);



    // we don't set height for properties panel, we let it fill out whatever is left
    //const float propertiesPanelHeight = getEditorDockBounds().height * 0.50f;

    enum PropertyFloatField
    {
        PROPERTY_POSITION_X = 0,
        PROPERTY_POSITION_Y,
        PROPERTY_POSITION_Z,

        PROPERTY_ROTATION_X,
        PROPERTY_ROTATION_Y,
        PROPERTY_ROTATION_Z,

        PROPERTY_SCALE_X,
        PROPERTY_SCALE_Y,
        PROPERTY_SCALE_Z,

        PROPERTY_LIGHT_INTENSITY,
        PROPERTY_LIGHT_RADIUS,

        PROPERTY_FLOAT_FIELD_COUNT
    };

    struct floatFieldState
    {
        char text[32] = {};
    };

    static floatFieldState propertyFloatFields[PROPERTY_FLOAT_FIELD_COUNT];
    static int activeFloatField = -1;
    static Vector2 cameraPanelScroll = { 0.0f, 0.0f };
    static Vector2 workspacePanelScroll = { 0.0f, 0.0f };
    static Vector2 propertiesPanelScroll = { 0.0f, 0.0f };

    enum CameraPropertyField
    {
        CAMERA_PROPERTY_FOV = 0,
        CAMERA_PROPERTY_MOVE_SPEED,
        CAMERA_PROPERTY_SENSITIVITY,
        CAMERA_PROPERTY_NEAR,
        CAMERA_PROPERTY_FAR,
        CAMERA_PROPERTY_FIELD_COUNT
    };

    static floatFieldState cameraPropertyFields[CAMERA_PROPERTY_FIELD_COUNT];
    static int activeCameraPropertyField = -1;

    static bool propertyColorEditMode[4] = { false, false, false, false };

    static bool propertyMaterialDropdownOpen = false;
    static bool viewportClickCandidate = false;
    static Vector2 viewportClickStart = { 0.0f, 0.0f };

    static bool isPropertyEditorActive()
    {
        if (propertyMaterialDropdownOpen) return true;
        if (activeFloatField != -1) return true;
        if (activeCameraPropertyField != -1) return true;
        if (freeDrawState.viewDropdownOpen) return true;
        return false;
    }

    // converts a string to a float, returns false if the string is not a valid float representation
    static bool tryParseFloat(const char* text, float& result)
    {
        if (text == nullptr || text[0] == '\0') return false;

        char* end = nullptr;
        // The strtof function converts a C-style string to a floating-point number (float) and updates a pointer to the character following the last character used in the conversion.
        // end tells you where the conversion stopped, so you can check if the entire string was a valid float representation.
        // parsed is our result of the conversion
        float parsed = std::strtof(text, &end);

        if (end == text) return false;  // no text changes occured

        while (*end == ' ' || *end == '\t') end++; // skip trailing whitespace

        if (*end != '\0') return false; // if there are any non-whitespace characters left, the string is not a valid float representation

        if (!std::isfinite(parsed)) return false; // if the parsed value is not finite (i.e., it's NaN or infinity), return false

        result = parsed;
        return true;
    }

    static void setFloatFieldText(floatFieldState& field, float value) {
        std::snprintf(field.text, sizeof(field.text), "%.3f", value);
    }

    static void resetPropertyEditorState()
    {
        activeFloatField = -1;

        for (auto& field : propertyFloatFields)
        {
            field.text[0] = '\0';
        }

        for (bool& editMode : propertyColorEditMode)
        {
            editMode = false;
        }

        propertyMaterialDropdownOpen = false;
    }

    static shape* propertyBoundObject = nullptr;

    void updatePropertyBinding(shape* object)
    {
        if (propertyBoundObject == object) return;

        propertyBoundObject = object;
        resetPropertyEditorState();
    }

    static bool drawFloatPropertyField(Rectangle bounds, int fieldIndex, float& value, float minimum, float maximum)
    {
        if (!(fieldIndex >= 0 && fieldIndex < PROPERTY_FLOAT_FIELD_COUNT))
        {
            return 0;
        }
        floatFieldState& field = propertyFloatFields[fieldIndex];

        if (activeFloatField != fieldIndex)
        {
            setFloatFieldText(field, value);
        }

        bool editing = (activeFloatField == fieldIndex);

        int result = GuiTextBox(
            bounds,
            field.text,
            static_cast<int>(sizeof(field.text)),
            editing
        );

        bool changed = false;

        // Update the actual object while the user types.
        if (editing)
        {
            float parsedValue = value;

            if (tryParseFloat(field.text, parsedValue))
            {
                parsedValue = Clamp(parsedValue, minimum, maximum);

                if (fabsf(parsedValue - value) > 0.0001f)
                {
                    value = parsedValue;
                    changed = true;
                }
            }
        }

        // raygui returns a result when entering/leaving edit mode.
        if (result != 0)
        {
            if (editing)
            {
                activeFloatField = -1;          // Finished editing
            }
            else
            {
                activeFloatField = fieldIndex;  // Start editing this field
            }

            editing = (activeFloatField == fieldIndex);
        }


        // Clicking outside the field finishes editing.
        if (editing && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), bounds))
        {
            float parsedValue = value;

            if (tryParseFloat(field.text, parsedValue))
            {
                value = Clamp(parsedValue, minimum, maximum);

                changed = true;
            }

            activeFloatField = -1;
            setFloatFieldText(field, value);
        }

        return changed;
    }
    // Self-contained twin of drawFloatPropertyField for camera settings — kept separate so editing a camera field doesn't get reset by updatePropertyBinding() firing on shape-selection changes.
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

    // A thin horizontal rule with consistent breathing room, used between property groups.
    static void drawSectionDivider(float x, float& y, float width)
    {
        using namespace UiStyle;
        y += kDividerMargin;
        Color line = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), 1, Fade(line, 0.6f));
        y += kDividerMargin;
    }

    // A muted, slightly smaller section label with a small accent tick to its left —
    // distinguishes "Position/Rotation/Scale" group headers from body text.
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

        drawSectionLabel(x, y, title);   // replaces the old raw DrawTextEx + manual y += 19.0f

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

            // Accent border around whichever field is currently being edited.
            bool isActive = (activeFloatField == firstFieldIndex + i);
            changed |= drawFloatPropertyField(fieldBounds, firstFieldIndex + i, *components[i], minimum, maximum);
            if (isActive) {
                DrawRectangleLinesEx({ fieldBounds.x - 1, fieldBounds.y - 1, fieldBounds.width + 2, fieldBounds.height + 2 }, 1.5f, kAccent);
            }
        }
        y += fieldHeight + kFieldGap;
        return changed;
    }

    static void drawColorProperty(
        float x, float& y, float width, Color& color)
    {
        DrawTextEx(GuiGetFont(), "Color", { x, y }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

        y += 19.0f;

        int channels[4] =
        {
            static_cast<int>(color.r),
            static_cast<int>(color.g),
            static_cast<int>(color.b),
            static_cast<int>(color.a)
        };

        const char* labels[4] =
        { "R", "G", "B", "A" };

        constexpr float gap = 5.0f;

        const float fieldWidth = (width - gap * 3.0f) / 4.0f;

        for (int i = 0; i < 4; i++)
        {
            Rectangle fieldBounds = {
                x + i * (fieldWidth + gap), y, fieldWidth, editorControlHeight };

            bool wasEditing = propertyColorEditMode[i];

            int result = GuiValueBox(fieldBounds, labels[i], &channels[i], 0, 255, wasEditing);

            if (result != 0) {
                propertyColorEditMode[i] = !wasEditing;
            }

            if (propertyColorEditMode[i] && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), fieldBounds)) {
                propertyColorEditMode[i] = false;
            }

            channels[i] = std::clamp(channels[i], 0, 255);
        }

        color.r = static_cast<unsigned char>(channels[0]);

        color.g = static_cast<unsigned char>(channels[1]);

        color.b = static_cast<unsigned char>(channels[2]);

        color.a = static_cast<unsigned char>(channels[3]);

        y += editorControlHeight + 10.0f;
    }



    bool isPointerOverEditorUi()
    {
        return CheckCollisionPointRec(GetMousePosition(), getEditorDockBounds());
    }

    // Draws the space for the panel
    static bool drawPanelFrame(Rectangle bounds, const char* title, bool open)
    {
        using namespace UiStyle;

        const float toggleWidth = 22.0f;
        const float collapsedWidth = 140.0f; // enough for a short title + toggle

        Rectangle headerBounds = bounds;
        if (!open)
        {
            headerBounds.width = collapsedWidth;
            headerBounds.x = bounds.x + bounds.width - collapsedWidth; // hug the right edge
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
        {

                freeDrawState.workspacePanelOpen = !freeDrawState.workspacePanelOpen;
        }

        if (!freeDrawState.workspacePanelOpen)
        {
            if (!interactive) GuiEnable();
            return;
        }

        const float rowHeight = static_cast<float>(std::max(26, GuiGetStyle(DEFAULT, TEXT_SIZE) + 12));
        const float headerH = 39.0f;

        Rectangle scrollBounds = { panel.x, panel.y + headerH, panel.width, panel.height - headerH };

        int objectCount = static_cast<int>(objects.size());
        float contentHeight = std::max(scrollBounds.height, objectCount * (rowHeight + 1.0f) + 10.0f);

        Rectangle content = { 0, 0, scrollBounds.width - 16.0f, contentHeight };
        Rectangle panelView;
        GuiScrollPanel(scrollBounds, NULL, content, &workspacePanelScroll, &panelView);

        BeginScissorMode(panelView.x, panelView.y, panelView.width, panelView.height);


        float y = scrollBounds.y + workspacePanelScroll.y + 4.0f;

        if (objectCount == 0)
        {
            DrawTextEx(GuiGetFont(), "Workspace is empty", { panel.x + 20.0f, y + 3.0f }, fontSize, spacing,
                GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
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

        EndScissorMode();

        if (!interactive) GuiEnable();
    }

    static void drawCameraPanel(bool interactive)
    {
        if (!(interactive || freeDrawState.viewDropdownOpen)) GuiDisable();

        Rectangle panel = getCameraPanelBounds();
        if (drawPanelFrame(panel, "Camera", freeDrawState.cameraPanelOpen))
        {

                freeDrawState.cameraPanelOpen = !freeDrawState.cameraPanelOpen;
        }

        if (!freeDrawState.cameraPanelOpen)
        {
            if (!interactive) GuiEnable();
            return;
        }

        const float contentX = panel.x + 10.0f;
        const float contentWidth = panel.width - 20.0f;
        const float controlHeight = editorControlHeight;

        float pinnedY = panel.y + 39.0f;

        // --- Pinned: projection label + view dropdown ---
        DrawTextEx(GuiGetFont(), TextFormat("Projection: %s", getEditableCamera().getCameraProjection()),
            { contentX, pinnedY + 5.0f }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
        pinnedY += 24.0f;

        DrawTextEx(GuiGetFont(), "View", { contentX, pinnedY + 5.0f }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

        Rectangle viewRect = { contentX + 45.0f, pinnedY, contentWidth - 45.0f, controlHeight };
        const char* viewOptions = "Free;Front;Top;Left;Right";
        static int activeViewIndex = static_cast<int>(freeDrawState.currentViewIndex);

        // Keep the static synced if currentViewIndex changed elsewhere (e.g. re-entering the scene resets it in freeDrawInit, but this static only initializes once ever).
        if (!freeDrawState.viewDropdownOpen && activeViewIndex != static_cast<int>(freeDrawState.currentViewIndex))
            activeViewIndex = static_cast<int>(freeDrawState.currentViewIndex);

        if (GuiDropdownBox(viewRect, viewOptions, &activeViewIndex, freeDrawState.viewDropdownOpen))
            freeDrawState.viewDropdownOpen = !freeDrawState.viewDropdownOpen;

        // Close on outside click, but don't close on a click that's actually landing in the open list itself (which renders just below viewRect).
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

        // Don't draw anything under an open dropdown list — same guard pattern as the material dropdown in drawPropertiesPanel.
        if (freeDrawState.viewDropdownOpen)
        {
            if (!interactive) GuiEnable();
            return;
        }

        // --- Scrollable: camera settings + split screen toggle ---
        Rectangle scrollBounds = { panel.x, pinnedY, panel.width, panel.y + panel.height - pinnedY };
        if (scrollBounds.height < 20.0f) scrollBounds.height = 20.0f; // never let it invert

        const float fieldBlockHeight = 44.0f; // label + box + gap, per field
        const float contentHeight = 20.0f + fieldBlockHeight * CAMERA_PROPERTY_FIELD_COUNT + 30.0f + controlHeight + 20.0f;

        Rectangle content = { 0, 0, scrollBounds.width - 16.0f, contentHeight };
        Rectangle panelView;
        GuiScrollPanel(scrollBounds, NULL, content, &cameraPanelScroll, &panelView);

        BeginScissorMode(panelView.x, panelView.y, panelView.width, panelView.height);

        float y = scrollBounds.y + cameraPanelScroll.y + 8.0f;
        float fx = contentX;
        float fw = contentWidth - 16.0f; // leave room for the scrollbar

        drawSectionLabel(fx, y, "Camera Settings");

        cameraController* cam;

        struct { const char* label; float* value; float minimum; float maximum; int index; } 
        camFields[] = {
            { "FOV", &getEditableCamera().getFovy(), 5.0f, 120.0f, CAMERA_PROPERTY_FOV },
            { "Move Speed",  &getEditableCamera().getWalkSpeed(),        0.01f,  1000.0f,    CAMERA_PROPERTY_MOVE_SPEED},
            { "Sensitivity", &getEditableCamera().getMouseSensitivity(), 0.001f,   10.0f,    CAMERA_PROPERTY_SENSITIVITY},
            //{ "Near",        &getEditableCamera().getNearPlane(),        0.001f,  100.0f,    CAMERA_PROPERTY_NEAR},
            //{ "Far",         &getEditableCamera().farPlane,         1.0f,  100000.0f,   CAMERA_PROPERTY_FAR },
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
        GuiToggle(splitBtn, splitActive ? "Split Screen: On" : "Split Screen: Off", & splitActive);
        
        freeDrawState.splitScreenEnabled = splitActive;

        if (splitActive && freeDrawState.activeViews.size() == 1)
        {
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
            for (size_t i = 1; i < freeDrawState.activeViews.size(); ++i)
                freeDrawState.activeViews[i].releaseTarget();

            freeDrawState.activeViews.resize(1);
        }

        
        y += controlHeight + 12.0f;

        EndScissorMode();

        if (!interactive) GuiEnable();
    }

    // include light object selection later on
    void drawPropertiesPanel(bool interactive)
    {
        if (!interactive) GuiDisable();
        const int total = static_cast<int>(selectedObjects.size());

        Rectangle panel = getPropertiesPanelBounds();
        if (drawPanelFrame(panel, "Properties", freeDrawState.propertiesPanelOpen))
        {

                freeDrawState.propertiesPanelOpen = !freeDrawState.propertiesPanelOpen;
        }

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
            resetPropertyEditorState();
            DrawTextEx(GuiGetFont(), "No object selected", { contentX, y }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
            if (!interactive) GuiEnable();
            return;
        }

        if (total > 1)
        {
            resetPropertyEditorState();
            DrawTextEx(GuiGetFont(), TextFormat("Multiple objects selected: %d", total), { contentX, y }, fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
            if (!interactive) GuiEnable();
            return;
        }

        shape* selected = activeObject;
        if (selected == nullptr) { if (!interactive) GuiEnable(); return; }

        updatePropertyBinding(selected);

        DrawTextEx(GuiGetFont(), TextFormat("%s %d", selected->getObjectTypeString(), selected->getId()), { contentX, y }, fontSize + 2.0f, spacing, UiStyle::kAccent);
        y += 25.0f;

        // --- Pinned: material dropdown (kept outside the scroll so its open list is never clipped) ---
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
            if (!interactive) GuiEnable();  // <- fixes the stuck-disabled bug from before
            return;
        }

        // --- Scrollable: Position / Rotation / Scale ---
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

        // --- Pinned: deselect button ---
        Rectangle deselectButton = { contentX, panel.y + panel.height - editorControlHeight - 10.0f, 110.0f, editorControlHeight };
        if (GuiButton(deselectButton, "Deselect"))
        {
            selectedObjects.clear();
            for (auto& object : objects) object->setSelected(false);
            resetPropertyEditorState();
        }

        if (!interactive) GuiEnable();
    }
}

void freeDrawInit() {
	TraceLog(LOG_INFO, "Initializing Free Draw Mode Scene");
    TraceLog(LOG_INFO, "%d", static_cast<int>(currentScene));

	if (freeDrawState.initiliased) return; // Prevent reinitialization if already initialized
    InitTransformGizmo(); // Initialize the transform gizmo, only needs to be called once
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

    if (!freeDrawState.initiliased) return; // Prevent update if not initialized

    // Compute once, share across click-ray and camera-update logic below.
    std::vector<Rectangle> bounds = computeViewportBounds(static_cast<int>(freeDrawState.activeViews.size()));

    bool usingGizmo = updateObjectTransformGizmo(getEditableCamera().getCamera());
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        viewportClickStart = GetMousePosition();
        viewportClickCandidate = !usingGizmo && !isPointerOverEditorUi() &&
            !freeDrawState.mouseButtonPressed;
    }

    if (viewportClickCandidate && IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        Vector2Distance(viewportClickStart, GetMousePosition()) > 4.0f)
    {
        viewportClickCandidate = false;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (viewportClickCandidate && !isPointerOverEditorUi() &&
            !freeDrawState.mouseButtonPressed)
        {
            // Find which viewport the mouse is actually over — in split screen this
            // isn't necessarily slot 0, and the ray math is wrong if we assume it is.
            Vector2 mouse = GetMousePosition();
            int hitIndex = -1;
            for (size_t i = 0; i < bounds.size(); ++i)
            {
                if (CheckCollisionPointRec(mouse, bounds[i]))
                {
                    hitIndex = static_cast<int>(i);
                    break;
                }
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

                selectObjectByRay(ray);
            }
        }
        viewportClickCandidate = false;
    }

    if (IsKeyPressed(KEY_F1))
    {
        freeDrawState.helpTip = !freeDrawState.helpTip;
    }

    if (IsKeyPressed(KEY_DELETE) && !isPropertyEditorActive())
    {
        deleteObjects();
        resetPropertyEditorState();
    }

    if (!usingGizmo && !isPointerOverEditorUi())
    {
        Vector2 mouse = GetMousePosition();

        for (size_t i = 0; i < freeDrawState.activeViews.size(); ++i)
        {
            ViewportSlot& slot = freeDrawState.activeViews[i];

            if (!slot.editable && slot.trackSelection)
            {
                Vector3 focus = activeObject ? activeObject->getTransform().translation : Vector3{ 0,0,0 };
                slot.camera.setView(slot.presetView, focus);
            }

            if (slot.editable && !freeDrawState.cameraLocked && CheckCollisionPointRec(mouse, bounds[i]))
            {
                slot.camera.updateCamera();
            }
        }
    }
}

void freeDrawDraw() {

    std::vector<Rectangle> viewBounds = computeViewportBounds(static_cast<int>(freeDrawState.activeViews.size()));

    for (size_t i = 0; i < freeDrawState.activeViews.size(); ++i)
    {
        ViewportSlot& slot = freeDrawState.activeViews[i];
        Rectangle vb = viewBounds[i];

        // Cheap no-op most frames — only reallocates when this slot's pixel size
        // actually changed (window resize, split-screen toggle, layout change).
        slot.ensureTarget(static_cast<int>(vb.width), static_cast<int>(vb.height));

        DrawCameraScene(slot.camera.getCamera(), vb, slot.target);
    }

    // Top-right options (gear) button to open Options menu
    const float iconSize = 32.0f;
    Rectangle btnOptionsIcon = { (float)GetScreenWidth() - iconSize - 10.0f, 10.0f, iconSize, iconSize };
    // Use a GuiButton for click detection, draw a gear-like icon on top to match rayGUI style
    if (GuiButton(btnOptionsIcon, "")) {
        sceneManagerChangeScene(sceneId::SCENE_OPTIONS);
    }
    GuiDrawIcon(ICON_GEAR_BIG, btnOptionsIcon.x, btnOptionsIcon.y, 2, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

    //topBar(currentResIndex, freeDrawState.dropdownEditmode);

    if ((IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !isPointerOverEditorUi()) || freeDrawState.mouseButtonPressed) {
        contextMenu(freeDrawState.mouseButtonPressed, getEditableCamera().getCamera()); // under InputHandler.cpp
    }

    // Properties tab � middle right
    drawPropertiesPanel(CheckCollisionPointRec(GetMousePosition(), getPropertiesPanelBounds()));
    drawWorkspacePanel(CheckCollisionPointRec(GetMousePosition(), getWorkspacePanelBounds()));
    drawCameraPanel(CheckCollisionPointRec(GetMousePosition(), getCameraPanelBounds()));


    // Draw camera controller settings overlay for user reference
    if (freeDrawState.helpTip)
    {
        drawCameraControllerSettings();
    }
}

void freeDrawUnload() {
    freeDrawState.initiliased = false;
    UnloadTransformGizmo();

    for (auto& slot : freeDrawState.activeViews)
        slot.releaseTarget();
    // Models, material textures, and lighting are application-wide resources
    // initialized once by sceneManagerInit(). Destroying them here made a
    // second visit to Learn use invalid GPU resources and crash.
}
