#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "raylib.h"

void contextMenu(bool& mouseButtonPressed, Camera3D& camera, Rectangle, bool guidedRestricted = false);

enum {
    STATE_BASE = 0,
    STATE_SHOW_MENU,
    STATE_SHOW_SUBMENU,
};

#endif // INPUT_HANDLER_H
