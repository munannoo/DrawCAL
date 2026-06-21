// theme file headers
// add themes to assets/themes
// themes may be generated from rayguistyler (rgs)
	
#ifndef THEME_H
#define THEME_H

#include "raygui.h"


// Declaring theme choices, add more if desired
enum class themes {DEFAULT = 0, DARK, CYBER, GENESIS, JUNGLE, AMBER};
extern const char* themeOptions;
extern int currentThemeIndex, lastThemeIndex;

void GuiLoadStyleDark(void);
void GuiLoadStyleCyber(void);
void GuiLoadStyleGenesis(void);
void GuiLoadStyleJungle(void);
void GuiLoadStyleAmber(void);



#endif