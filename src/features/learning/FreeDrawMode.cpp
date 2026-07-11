#include "FreeDrawMode.h"
#include "raylib.h"
#include "raygui.h"

static bool guidedWorkspace = false;

void SetGuidedWorkspace(bool guided)
{
    guidedWorkspace = guided;
    if (guided) freeDrawState.mouseButtonPressed = false;
}

namespace
{
    static void DrawEditorText(const char* text, int x, int y, int fontSize, Color color)
    {
        // GuiGetFont() follows GuiLoadStyle*(), so custom editor labels use the
        // same typeface as buttons, fields, and dropdowns in every theme.
        const float readableSize = static_cast<float>(fontSize);
        const float spacing = static_cast<float>(std::max(0, GuiGetStyle(DEFAULT, TEXT_SPACING)));
        DrawTextEx(GuiGetFont(), text,
                   { static_cast<float>(x), static_cast<float>(y) },
                   readableSize, spacing, color);
    }

    static float GetEditorControlHeight()
    {
        return static_cast<float>(std::max(27, GuiGetStyle(DEFAULT, TEXT_SIZE) + 11));
    }

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

    struct FloatFieldState
    {
        char text[32] = {};
        bool editMode = false;
    };

    static FloatFieldState propertyFloatFields[
        PROPERTY_FLOAT_FIELD_COUNT
    ];

    static bool propertyColorEditMode[4] =
    {
        false,
        false,
        false,
        false
    };

    static bool propertyMaterialDropdownOpen = false;
    static bool viewportClickCandidate = false;
    static Vector2 viewportClickStart = { 0.0f, 0.0f };

    static bool IsPropertyEditorActive()
    {
        if (propertyMaterialDropdownOpen) return true;
        for (const FloatFieldState& field : propertyFloatFields)
            if (field.editMode) return true;
        for (bool editMode : propertyColorEditMode)
            if (editMode) return true;
        return false;
    }

    static int propertyBoundType = -1;
    static int propertyBoundIndex = -1;

    static float ClampPropertyValue(
        float value,
        float minimum,
        float maximum
    )
    {
        return std::max(minimum, std::min(value, maximum));
    }

    static bool TryParseFloat(
        const char* text,
        float& result
    )
    {
        if (text == nullptr || text[0] == '\0')
        {
            return false;
        }

        char* end = nullptr;
        float parsed = std::strtof(text, &end);

        if (end == text)
        {
            return false;
        }

        while (*end == ' ' || *end == '\t')
        {
            end++;
        }

        if (*end != '\0')
        {
            return false;
        }

        if (!std::isfinite(parsed))
        {
            return false;
        }

        result = parsed;
        return true;
    }

    static void SetFloatFieldText(
        FloatFieldState& field,
        float value
    )
    {
        std::snprintf(
            field.text,
            sizeof(field.text),
            "%.3f",
            value
        );
    }

    static void ResetPropertyEditorState()
    {
        for (int i = 0;
             i < PROPERTY_FLOAT_FIELD_COUNT;
             i++)
        {
            propertyFloatFields[i].editMode = false;
            propertyFloatFields[i].text[0] = '\0';
        }

        for (bool& editMode : propertyColorEditMode)
        {
            editMode = false;
        }

        propertyMaterialDropdownOpen = false;
    }

    static void UpdatePropertyBinding(
        int objectType,
        int objectIndex
    )
    {
        if (propertyBoundType == objectType &&
            propertyBoundIndex == objectIndex)
        {
            return;
        }

        propertyBoundType = objectType;
        propertyBoundIndex = objectIndex;

        ResetPropertyEditorState();
    }

    static bool DrawFloatPropertyField(
        Rectangle bounds,
        int fieldIndex,
        float& value,
        float minimum,
        float maximum
    )
    {
        FloatFieldState& field =
            propertyFloatFields[fieldIndex];

        if (!field.editMode)
        {
            SetFloatFieldText(field, value);
        }

        bool wasEditing = field.editMode;

        int result = GuiTextBox(
            bounds,
            field.text,
            static_cast<int>(sizeof(field.text)),
            wasEditing
        );

        bool changed = false;

        // Update the actual object while the user types.
        if (wasEditing)
        {
            float parsedValue = value;

            if (TryParseFloat(field.text, parsedValue))
            {
                parsedValue = ClampPropertyValue(
                    parsedValue,
                    minimum,
                    maximum
                );

                if (parsedValue != value)
                {
                    value = parsedValue;
                    changed = true;
                }
            }
        }

        // raygui returns a result when entering/leaving edit mode.
        if (result != 0)
        {
            field.editMode = !wasEditing;
        }

        // Clicking outside the field finishes editing.
        if (field.editMode &&
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
            !CheckCollisionPointRec(GetMousePosition(), bounds))
        {
            float parsedValue = value;

            if (TryParseFloat(field.text, parsedValue))
            {
                value = ClampPropertyValue(
                    parsedValue,
                    minimum,
                    maximum
                );

                changed = true;
            }

            field.editMode = false;
            SetFloatFieldText(field, value);
        }

        return changed;
    }

    static void DrawVector3Property(
        const char* title,
        float x,
        float& y,
        float width,
        Vector3& value,
        int firstFieldIndex,
        float minimum,
        float maximum
    )
    {
        DrawEditorText(
            title,
            static_cast<int>(x),
            static_cast<int>(y),
            14,
            GetColor(
                GuiGetStyle(
                    DEFAULT,
                    TEXT_COLOR_NORMAL
                )
            )
        );

        y += 19.0f;

        constexpr float gap = 6.0f;
        constexpr float labelWidth = 14.0f;
        const float fieldHeight = GetEditorControlHeight();

        const float groupWidth =
            (width - gap * 2.0f) / 3.0f;

        const char* labels[3] =
        {
            "X",
            "Y",
            "Z"
        };

        float* components[3] =
        {
            &value.x,
            &value.y,
            &value.z
        };

        for (int i = 0; i < 3; i++)
        {
            const float groupX =
                x + i * (groupWidth + gap);

            DrawEditorText(
                labels[i],
                static_cast<int>(groupX),
                static_cast<int>(y + 5.0f),
                12,
                GetColor(
                    GuiGetStyle(
                        DEFAULT,
                        TEXT_COLOR_NORMAL
                    )
                )
            );

            Rectangle fieldBounds =
            {
                groupX + labelWidth,
                y,
                groupWidth - labelWidth,
                fieldHeight
            };

            DrawFloatPropertyField(
                fieldBounds,
                firstFieldIndex + i,
                *components[i],
                minimum,
                maximum
            );
        }

        y += fieldHeight + 10.0f;
    }

    static void DrawColorProperty(
        float x,
        float& y,
        float width,
        Color& color
    )
    {
        DrawEditorText(
            "Color",
            static_cast<int>(x),
            static_cast<int>(y),
            14,
            GetColor(
                GuiGetStyle(
                    DEFAULT,
                    TEXT_COLOR_NORMAL
                )
            )
        );

        y += 19.0f;

        int channels[4] =
        {
            static_cast<int>(color.r),
            static_cast<int>(color.g),
            static_cast<int>(color.b),
            static_cast<int>(color.a)
        };

        const char* labels[4] =
        {
            "R",
            "G",
            "B",
            "A"
        };

        constexpr float gap = 5.0f;

        const float fieldWidth =
            (width - gap * 3.0f) / 4.0f;

        for (int i = 0; i < 4; i++)
        {
            Rectangle fieldBounds =
            {
                x + i * (fieldWidth + gap),
                y,
                fieldWidth,
                GetEditorControlHeight()
            };

            bool wasEditing =
                propertyColorEditMode[i];

            int result = GuiValueBox(
                fieldBounds,
                labels[i],
                &channels[i],
                0,
                255,
                wasEditing
            );

            if (result != 0)
            {
                propertyColorEditMode[i] =
                    !wasEditing;
            }

            if (propertyColorEditMode[i] &&
                IsMouseButtonPressed(
                    MOUSE_BUTTON_LEFT
                ) &&
                !CheckCollisionPointRec(
                    GetMousePosition(),
                    fieldBounds
                ))
            {
                propertyColorEditMode[i] = false;
            }

            channels[i] = std::clamp(
                channels[i],
                0,
                255
            );
        }

        color.r =
            static_cast<unsigned char>(channels[0]);

        color.g =
            static_cast<unsigned char>(channels[1]);

        color.b =
            static_cast<unsigned char>(channels[2]);

        color.a =
            static_cast<unsigned char>(channels[3]);

        y += GetEditorControlHeight() + 10.0f;
    }

    static int MaterialToPropertyIndex(
        MaterialType material
    )
    {
        switch (material)
        {
            case MATERIAL_CONCRETE:
                return 0;

            case MATERIAL_WOOD:
                return 1;

            case MATERIAL_PLASTIC:
                return 2;

            case MATERIAL_COBBLESTONE:
                return 3;

            case MATERIAL_BRICK:
                return 4;

            case MATERIAL_TILES:
                return 5;

            case MATERIAL_METAL:
                return 6;

            case MATERIAL_MARBLE:
                return 7;

            case MATERIAL_ASPHALT:
                return 8;

            default:
                return 0;
        }
    }

    static MaterialType PropertyIndexToMaterial(
        int index
    )
    {
        switch (index)
        {
            case 0:
                return MATERIAL_CONCRETE;

            case 1:
                return MATERIAL_WOOD;

            case 2:
                return MATERIAL_PLASTIC;

            case 3:
                return MATERIAL_COBBLESTONE;

            case 4:
                return MATERIAL_BRICK;

            case 5:
                return MATERIAL_TILES;

            case 6:
                return MATERIAL_METAL;

            case 7:
                return MATERIAL_MARBLE;

            case 8:
                return MATERIAL_ASPHALT;

            default:
                return MATERIAL_CONCRETE;
        }
    }

    static const char* GetObjectPropertyTypeName(
        int objectType,
        const ObjectInstance& object
    )
    {
        if (object.isLight)
        {
            return "Point Light";
        }

        switch (objectType)
        {
            case 1:
                return "Cube";

            case 2:
                return "Sphere";

            case 3:
                return "Cylinder";

            default:
                return "Unknown";
        }
    }

    static Rectangle GetEditorDockBounds()
    {
        // Keep vector values readable without consuming too much viewport at
        // lower resolutions.
        const float width = ClampPropertyValue(GetScreenWidth() * 0.30f, 370.0f, 430.0f);
        return { static_cast<float>(GetScreenWidth()) - width,
                 52.0f, width,
                 static_cast<float>(GetScreenHeight()) - 52.0f };
    }

    static bool IsPointerOverEditorUi()
    {
        return CheckCollisionPointRec(GetMousePosition(), GetEditorDockBounds());
    }

    static void DrawPanelFrame(Rectangle bounds, const char* title)
    {
        const Color background = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
        const Color border = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
        const Color text = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
        DrawRectangleRec(bounds, Fade(background, 0.97f));
        DrawRectangleLinesEx(bounds, 1.0f, border);
        const Color header = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));
        DrawRectangle(static_cast<int>(bounds.x), static_cast<int>(bounds.y),
                      static_cast<int>(bounds.width), 32, header);
        DrawEditorText(title, static_cast<int>(bounds.x + 10.0f),
                 static_cast<int>(bounds.y + 7.0f), 16, text);
    }

    static void DrawWorkspacePanel()
    {
        Rectangle dock = GetEditorDockBounds();
        Rectangle panel = { dock.x, dock.y, dock.width, dock.height * 0.42f };
        DrawPanelFrame(panel, "Workspace");

        const Color text = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
        const float rowHeight = static_cast<float>(std::max(26, GuiGetStyle(DEFAULT, TEXT_SIZE) + 12));
        float y = panel.y + 39.0f;
        DrawEditorText("v", static_cast<int>(panel.x + 10), static_cast<int>(y + 4), 13, text);
        DrawEditorText("Workspace", static_cast<int>(panel.x + 28), static_cast<int>(y + 4), 14, text);
        y += rowHeight;

        int visibleCount = 0;
        for (int type = 1; type <= 3; ++type)
        {
            for (int index = 0; index < getObjectCount(type); ++index)
            {
                ObjectInstance* object = getObjectMutable(type, index);
                if (object == nullptr) continue;
                Rectangle row = { panel.x + 20.0f, y, panel.width - 30.0f, rowHeight };
                const char* className = GetObjectPropertyTypeName(type, *object);
                const char* label = TextFormat("%s  %s %d", object->isLight ? "*" : "#", className, index + 1);
                const bool wasSelected = object->isSelected;
                bool rowSelected = wasSelected;
                const int previousAlignment = GuiGetStyle(TOGGLE, TEXT_ALIGNMENT);
                GuiSetStyle(TOGGLE, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
                GuiToggle(row, label, &rowSelected);
                GuiSetStyle(TOGGLE, TEXT_ALIGNMENT, previousAlignment);
                if (rowSelected != wasSelected)
                {
                    const bool additive = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
                    selectObject(type, index, additive);
                }
                y += rowHeight + 1.0f;
                ++visibleCount;
                if (y + rowHeight > panel.y + panel.height) break;
            }
            if (y + rowHeight > panel.y + panel.height) break;
        }

        if (visibleCount == 0)
            DrawEditorText("Workspace is empty", static_cast<int>(panel.x + 28), static_cast<int>(y + 3),
                     13, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
    }
}

void freeDrawInit() {
	TraceLog(LOG_INFO, "Initializing Free Draw Mode Scene");
    TraceLog(LOG_INFO, "%d", static_cast<int>(currentScene));
	if (freeDrawState.initiliased) return; // Prevent reinitialization if already initialized
    InitTransformGizmo(); // Initialize the transform gizmo, only needs to be called once
	InitCamera(freeDrawState.camera);
    freeDrawState.drawArea = { 200,140,220,44 };
	freeDrawState.initiliased = true;
	freeDrawState.mouseButtonPressed = false;
    freeDrawState.currentViewIndex = VIEW_FREE;
    freeDrawState.lastViewIndex = VIEW_FREE;
    freeDrawState.viewDropdownOpen = false;
    freeDrawState.cameraLocked = false;
    freeDrawState.helpTip = false;
}

void freeDrawUpdate() {

	if (!freeDrawState.initiliased) return; // Prevent update if not initialized
    bool usingGizmo = updateObjectTransformGizmo(freeDrawState.camera);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        viewportClickStart = GetMousePosition();
        viewportClickCandidate = !usingGizmo && !IsPointerOverEditorUi() &&
                                 !freeDrawState.mouseButtonPressed;
    }

    if (viewportClickCandidate && IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        Vector2Distance(viewportClickStart, GetMousePosition()) > 4.0f)
    {
        viewportClickCandidate = false;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        if (viewportClickCandidate && !IsPointerOverEditorUi() &&
            !freeDrawState.mouseButtonPressed)
        {
            Ray ray = GetScreenToWorldRay(GetMousePosition(), freeDrawState.camera);
            leftclick(ray);
        }
        viewportClickCandidate = false;
    }

    // Allow zoom with mouse wheel when camera is locked (preset view)
    if (freeDrawState.cameraLocked && !IsPointerOverEditorUi()) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            freeDrawState.camera.fovy *= (1.0f - wheel * 0.1f);
            if (freeDrawState.camera.fovy < 5.0f) freeDrawState.camera.fovy = 5.0f;
            if (freeDrawState.camera.fovy > 120.0f) freeDrawState.camera.fovy = 120.0f;
        }
    }

    if (IsKeyPressed(KEY_F1))
    {
        freeDrawState.helpTip = !freeDrawState.helpTip;
    }

    if (!guidedWorkspace && IsKeyPressed(KEY_BACKSPACE) &&
        !IsPropertyEditorActive())
    {
        deleteobj();
        propertyBoundType = -1;
        propertyBoundIndex = -1;
        ResetPropertyEditorState();
    }



    //if (currentResIndex != lastResIndex) {
    //    SetWindowSize(cr[currentResIndex].width, cr[currentResIndex].height);
    //    lastResIndex = currentResIndex;
    //}

    if (!usingGizmo && !IsPointerOverEditorUi()) {
        if (!freeDrawState.cameraLocked) UpdateCameraController(freeDrawState.camera);
    }




}

void freeDrawDraw() {
    DrawCameraScene(freeDrawState.camera);
    // Top-right options (gear) button to open Options menu
    const float iconSize = 32.0f;
    Rectangle btnOptionsIcon = { (float)GetScreenWidth() - iconSize - 10.0f, 10.0f, iconSize, iconSize };
    // Use a GuiButton for click detection, draw a gear-like icon on top to match rayGUI style
    if (GuiButton(btnOptionsIcon, "")) {
        sceneManagerChangeScene(sceneId::SCENE_OPTIONS);
    }
    GuiDrawIcon(ICON_GEAR_BIG, btnOptionsIcon.x, btnOptionsIcon.y, 2,
                GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

    //topBar(currentResIndex, freeDrawState.dropdownEditmode);

    changeCameraView();

    if (!guidedWorkspace &&
        ((IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !IsPointerOverEditorUi()) ||
         freeDrawState.mouseButtonPressed)) {
        contextMenu(freeDrawState.mouseButtonPressed, freeDrawState.camera); // under InputHandler.cpp
    }

    // Properties tab � middle right
    DrawWorkspacePanel();
    getProperties();
    

    // Draw camera controller settings overlay for user reference
    if (freeDrawState.helpTip)
    {
        drawCameraControllerSettings();
    }
}

void freeDrawUnload() {
    freeDrawState.initiliased = false;
    UnloadTransformGizmo();
    // Models, material textures, and lighting are application-wide resources
    // initialized once by sceneManagerInit(). Destroying them here made a
    // second visit to Learn use invalid GPU resources and crash.
}

void changeCameraView() {
    // Top-right view dropdown
    const int viewW = 140;
    const int viewH = 30;
    Rectangle viewRect = { (float)GetScreenWidth() / (float)2 - viewW / 2, viewH, (float)viewW, (float)viewH };
    const char* viewOptions = "Free;Front;Top;Left;Right";
    static int activeViewIndex = static_cast<int>(freeDrawState.currentViewIndex);

    // Dropdown box, main key
    if (GuiDropdownBox(viewRect, viewOptions, &activeViewIndex, freeDrawState.viewDropdownOpen)) {
        freeDrawState.viewDropdownOpen = !freeDrawState.viewDropdownOpen;
    }

    freeDrawState.currentViewIndex = static_cast<viewIndex>(activeViewIndex);

    // If view selection changed, apply camera preset and lock camera movement
    if (freeDrawState.currentViewIndex != freeDrawState.lastViewIndex) {
        // Apply presets based on selection (0 = Free/unlocked)
        switch (freeDrawState.currentViewIndex) {
            case VIEW_FREE: // Free - restore default controller camera
                InitCamera(freeDrawState.camera);
                freeDrawState.cameraLocked = false;
                break;
            case VIEW_FRONT: // Front
                freeDrawState.camera.position = { 0.0f, 0.0f, 10.0f };
                freeDrawState.camera.target = { 0.0f, 0.0f, 0.0f };
                freeDrawState.camera.up = { 0.0f, 1.0f, 0.0f };
                freeDrawState.camera.projection = CAMERA_PERSPECTIVE;
                freeDrawState.camera.fovy = 45.0f;
                freeDrawState.cameraLocked = true;
                break;
            case VIEW_TOP: // Top
                freeDrawState.camera.position = { 0.0f, 10.0f, 0.0f };
                freeDrawState.camera.target = { 0.0f, 0.0f, 0.0f };
                freeDrawState.camera.up = { 0.0f, 0.0f, -1.0f };
                freeDrawState.camera.projection = CAMERA_ORTHOGRAPHIC;
                freeDrawState.camera.fovy = 45.0f;
                freeDrawState.cameraLocked = true;
                break;
            case VIEW_LEFT: // Left
                freeDrawState.camera.position = { -10.0f, 0.0f, 0.0f };
                freeDrawState.camera.target = { 0.0f, 0.0f, 0.0f };
                freeDrawState.camera.up = { 0.0f, 1.0f, 0.0f };
                freeDrawState.camera.projection = CAMERA_PERSPECTIVE;
                freeDrawState.camera.fovy = 45.0f;
                freeDrawState.cameraLocked = true;
                break;
            case VIEW_RIGHT: // Right
                freeDrawState.camera.position = { 10.0f, 0.0f, 0.0f };
                freeDrawState.camera.target = { 0.0f, 0.0f, 0.0f };
                freeDrawState.camera.up = { 0.0f, 1.0f, 0.0f };
                freeDrawState.camera.projection = CAMERA_PERSPECTIVE;
                freeDrawState.camera.fovy = 45.0f;
                freeDrawState.cameraLocked = true;
                break;
			case VIEW_NONE:
			default:
                break;
        }
        freeDrawState.lastViewIndex = freeDrawState.currentViewIndex;
    }
}


void getProperties()
{
    const int total = getTotalSelectedCount();

    const Rectangle dock = GetEditorDockBounds();
    const float panelWidth = dock.width;
    const float panelY = dock.y + dock.height * 0.42f;
    const float panelHeight = GetScreenHeight() - panelY;
    const float panelX = dock.x;

    Rectangle panel =
    {
        panelX,
        panelY,
        panelWidth,
        panelHeight
    };

    DrawPanelFrame(panel, "Properties");

    const float contentX = panelX + 10.0f;
    const float contentWidth = panelWidth - 20.0f;

    float y = panelY + 39.0f;

    if (total == 0)
    {
        propertyBoundType = -1;
        propertyBoundIndex = -1;

        ResetPropertyEditorState();

        DrawEditorText(
            "No object selected",
            static_cast<int>(contentX),
            static_cast<int>(y),
            13,
            GetColor(
                GuiGetStyle(
                    DEFAULT,
                    TEXT_COLOR_DISABLED
                )
            )
        );

        return;
    }

    if (total > 1)
    {
        propertyBoundType = -1;
        propertyBoundIndex = -1;

        ResetPropertyEditorState();

        DrawEditorText(
            TextFormat(
                "Multiple objects selected: %d",
                total
            ),
            static_cast<int>(contentX),
            static_cast<int>(y),
            13,
            GetColor(
                GuiGetStyle(
                    DEFAULT,
                    TEXT_COLOR_NORMAL
                )
            )
        );

        return;
    }

    int selectedType = 0;
    int selectedIndex = -1;

    ObjectInstance* selected =
        getFirstSelectedMutable(
            &selectedType,
            &selectedIndex
        );

    if (selected == nullptr)
    {
        return;
    }

    UpdatePropertyBinding(
        selectedType,
        selectedIndex
    );

    const char* typeName =
        GetObjectPropertyTypeName(
            selectedType,
            *selected
        );

    DrawEditorText(
        TextFormat("%s %d", typeName, selectedIndex + 1),
        static_cast<int>(contentX),
        static_cast<int>(y),
        13,
        GetColor(
            GuiGetStyle(
                DEFAULT,
                TEXT_COLOR_NORMAL
            )
        )
    );

    y += 25.0f;

    if (!selected->isLight)
    {
        DrawEditorText("Material", static_cast<int>(contentX), static_cast<int>(y + 5.0f), 14,
                       GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

        int materialIndex = MaterialToPropertyIndex(selected->material);
        const float controlHeight = GetEditorControlHeight();
        Rectangle materialBounds = { contentX + 82.0f, y, contentWidth - 82.0f, controlHeight };
        if (GuiDropdownBox(materialBounds,
                           "Concrete;Wood;Plastic;Cobblestone;Brick;Tiles;Metal;Marble;Asphalt",
                           &materialIndex, propertyMaterialDropdownOpen))
        {
            propertyMaterialDropdownOpen = !propertyMaterialDropdownOpen;
        }
        selected->material = PropertyIndexToMaterial(materialIndex);
        y += controlHeight + 10.0f;

        // Keep the open list above the remaining property controls and prevent
        // clicks on its choices from also editing a field underneath it.
        if (propertyMaterialDropdownOpen) return;
    }

    DrawVector3Property(
        "Position",
        contentX,
        y,
        contentWidth,
        selected->position,
        PROPERTY_POSITION_X,
        -100000.0f,
        100000.0f
    );

    DrawVector3Property(
        "Rotation",
        contentX,
        y,
        contentWidth,
        selected->rotation,
        PROPERTY_ROTATION_X,
        -360000.0f,
        360000.0f
    );

    DrawVector3Property(
        "Scale",
        contentX,
        y,
        contentWidth,
        selected->scale,
        PROPERTY_SCALE_X,
        0.01f,
        10000.0f
    );

    DrawColorProperty(
        contentX,
        y,
        contentWidth,
        selected->color
    );

    if (selected->isLight)
    {
        DrawEditorText(
            "Light",
            static_cast<int>(contentX),
            static_cast<int>(y),
            14,
            GetColor(
                GuiGetStyle(
                    DEFAULT,
                    TEXT_COLOR_NORMAL
                )
            )
        );

        y += 19.0f;

        DrawEditorText(
            "Intensity",
            static_cast<int>(contentX),
            static_cast<int>(y + 5.0f),
            12,
            GetColor(
                GuiGetStyle(
                    DEFAULT,
                    TEXT_COLOR_NORMAL
                )
            )
        );

        DrawFloatPropertyField(
            Rectangle
            {
                contentX + 80.0f,
                y,
                contentWidth - 80.0f,
                GetEditorControlHeight()
            },
            PROPERTY_LIGHT_INTENSITY,
            selected->lightIntensity,
            0.0f,
            100000.0f
        );

        y += GetEditorControlHeight() + 8.0f;

        DrawEditorText(
            "Radius",
            static_cast<int>(contentX),
            static_cast<int>(y + 5.0f),
            12,
            GetColor(
                GuiGetStyle(
                    DEFAULT,
                    TEXT_COLOR_NORMAL
                )
            )
        );

        DrawFloatPropertyField(
            Rectangle
            {
                contentX + 80.0f,
                y,
                contentWidth - 80.0f,
                GetEditorControlHeight()
            },
            PROPERTY_LIGHT_RADIUS,
            selected->lightRadius,
            0.01f,
            100000.0f
        );

        y += GetEditorControlHeight() + 10.0f;

        // Immediately update the PBR light after editing
        // position, color, intensity, or radius.
        SyncObjectLightsToScene();
    }

    Rectangle deselectButton =
    {
        contentX,
        panelY + panelHeight - GetEditorControlHeight() - 10.0f,
        110.0f,
        GetEditorControlHeight()
    };

    if (GuiButton(deselectButton, "Deselect"))
    {
        deselectAllObjects();

        propertyBoundType = -1;
        propertyBoundIndex = -1;

        ResetPropertyEditorState();
        return;
    }
}
