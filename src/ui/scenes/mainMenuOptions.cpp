#include "mainMenuOptions.h"
#include "UiStyle.h"

std::string textResolution = "Resolution: ";

std::string textFullScreen = "Windowed";

std::string textVSync = "V-Sync: ON";

bool dropdownEditMode = false;

const char* resolutionOptions = "720p (HD);900p (HD+);1080p (Full HD);1440p (Quad HD);1600p (WQXGA)";


// Options Menu

void optionsMenuInit() {
	btnGraphics = { btnLeft, btnTop, btnWidth, btnHeight };
	btnControls = { btnLeft, btnTop + 1 * btnGap, btnWidth, btnHeight };
	btnInterface = { btnLeft, btnTop + 2 * btnGap, btnWidth, btnHeight };
	btnBack = { btnLeft, btnTop + 3 * btnGap, btnWidth, btnHeight };
}

void optionsMenuUpdate() {

}

void optionsMenuDraw() {
	UiStyle::LoadMinimalStyle();
	UiStyle::DrawBackground();
	UiStyle::DrawSceneHeader("Options", "Settings and preferences.", btnTop - 68.0f, btnTop - 28.0f);

	if (GuiButton(btnGraphics, "Display")) sceneManagerChangeScene(optionSceneId::OPTIONS_GRAPHICS);
	if (GuiButton(btnControls, "Controls")) sceneManagerChangeScene(optionSceneId::OPTIONS_CONTROLS);
	if (GuiButton(btnInterface, "UI")) sceneManagerChangeScene(optionSceneId::OPTIONS_INTERFACE); // Interface for ui scale, font size, themes, etc.

	if (GuiButton(btnBack, "Back to Main Menu")) {
		pendingScene = sceneId::SCENE_MENU;
		currentOptionScene = optionSceneId::OPTIONS_NONE;
	}
}

void optionsMenuUnload() {

}

// Options Graphics

// Placed for the sake of updating resolution inside the graphics section
static void optionsGraphicsInitialise() {
	btnResolution = { btnLeft, btnTop, btnWidth, btnHeight };
	btnFullScreen = { btnLeft, btnTop + 1 * btnGap, btnWidth, btnHeight };
	btnVSync = { btnLeft, btnTop + 2 * btnGap, btnWidth, btnHeight };
	btnBack = { btnLeft, btnTop + 3 * btnGap, btnWidth, btnHeight };
}

void optionsGraphicsInit() {	
	optionsGraphicsInitialise();
}

void optionsGraphicsDraw() {

	UiStyle::LoadMinimalStyle();
	UiStyle::DrawBackground();
	UiStyle::DrawSceneHeader("Display Settings", "Graphics and window mode.", btnTop - 68.0f, btnTop - 28.0f);

	if (GuiButton(btnResolution, textResolution.c_str())) dropdownEditMode = true;

	if (GuiDropdownBox(Rectangle{ btnResolution.x + btnWidth, btnResolution.y, btnResolution.width, btnResolution.height }, resolutionOptions, &currentResIndex, dropdownEditMode)) {
		dropdownEditMode = !dropdownEditMode;
	}

	if (!dropdownEditMode) {
		if (GuiButton(btnFullScreen, textFullScreen.c_str())) {
			ToggleFullscreen();

			if (IsWindowFullscreen() == 0) textFullScreen = "Windowed";
			else textFullScreen = "Fullscreen";

			changeButtonResolution();
			ShowCursor();
		}

		if (GuiButton(btnVSync, textVSync.c_str())) {
			if (IsWindowState(FLAG_VSYNC_HINT)) {
				ClearWindowState(FLAG_VSYNC_HINT);
				textVSync = "V-Sync: Off";
			}
			else {
				SetWindowState(FLAG_VSYNC_HINT);
				textVSync = "V-Sync: On";
			}
		}

		if (GuiButton(btnBack, "Back to Graphics Settings")) {
			pendingOptionScene = optionSceneId::OPTIONS_MENU;
		}
	}
}

void optionsGraphicsUpdate() {
	changeResolution();
}

void optionsGraphicsUnload() {

}

void changeResolution() {
	if (currentResIndex != lastResIndex) {
		resolutionClass selected = resolutions[currentResIndex];

		SetWindowSize(selected.width, selected.height);
		lastResIndex = currentResIndex;
		changeButtonResolution();
		optionsGraphicsInit(); // We need this because button size change is not registered until change of scene, so needed dynamicability for this scene
	}
}

// Controls

void optionsControlsInit(){
	btnBack = { btnLeft, btnTop + 2 * btnGap, btnWidth, btnHeight };
}
void optionsControlsUpdate(){}
void optionsControlsDraw(){
	UiStyle::LoadMinimalStyle();
	UiStyle::DrawBackground();
	UiStyle::DrawSceneHeader("Controls", "Camera and input help.", btnTop - 68.0f, 50.0f);

	drawCameraControllerSettings();
	if (GuiButton(btnBack, "Back")) {
		pendingOptionScene = optionSceneId::OPTIONS_MENU;
	}
}
void optionsControlsUnload(){}

// Interface

void optionsInterfaceInit() {
	btnUIScale = { btnLeft, btnTop, btnWidth, btnHeight };
	btnTheme = { btnLeft, btnTop + 1 * btnGap, btnWidth, btnHeight };
	btnFontSize = { btnLeft, btnTop + 2 * btnGap, btnWidth, btnHeight };
	btnBack = { btnLeft, btnTop + 3 * btnGap, btnWidth, btnHeight };
}

void optionsInterfaceUpdate() {
	changeTheme();
}

void optionsInterfaceDraw() {
	UiStyle::LoadMinimalStyle();
	UiStyle::DrawBackground();
	UiStyle::DrawSceneHeader("Interface Settings", "Customize your UI.", btnTop - 68.0f, btnTop - 28.0f);

	if (GuiButton(btnUIScale, "UI SCALE: ")) {
		// TODO: UI scale settings
	}
	
	if (GuiButton(btnTheme, "Theme: ")) dropdownEditMode = true;

	if (GuiDropdownBox(Rectangle{ btnTheme.x + btnWidth, btnTheme.y, btnTheme.width, btnTheme.height }, themeOptions, &currentThemeIndex, dropdownEditMode)) {
		dropdownEditMode = !dropdownEditMode;
	}

	if (!dropdownEditMode) {
		if (GuiButton(btnFontSize, "Font Size: ")) {
			// TODO: Font size settings
		}

		if (GuiButton(btnBack, "Back")) {
			pendingOptionScene = optionSceneId::OPTIONS_MENU;
		}
	}
}

void optionsInterfaceUnload(){}

void changeTheme() {
	if (currentThemeIndex != lastThemeIndex) {
		switch (currentThemeIndex) {
			case 0:	GuiLoadStyleDefault();  break;
			case 1: GuiLoadStyleDark(); TraceLog(LOG_INFO, "MyButton clicked"); break;
			case 2: GuiLoadStyleCyber(); break;
			case 3: GuiLoadStyleGenesis(); break;
			case 4: GuiLoadStyleJungle(); break;
			case 5: GuiLoadStyleAmber(); break;
			default: break;
		}
		lastThemeIndex = currentThemeIndex;
		changeButtonResolution();
		optionsInterfaceInit();
	}
}