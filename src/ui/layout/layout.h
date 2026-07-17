#ifndef LAYOUT_H
#define LAYOUT_H

#include "raylib.h"

enum viewIndex
{
    VIEW_NONE = -1,
    VIEW_FREE = 0,
    VIEW_FRONT,
    VIEW_TOP,
    VIEW_LEFT,
    VIEW_RIGHT
};

Rectangle GetEditorDockBounds();
Rectangle GetWorkspacePanelBounds(bool propertiesVisible);
Rectangle GetPropertiesPanelBounds(bool workspaceVisible);

bool IsPointerOverEditorLayout(bool workspaceVisible, bool propertiesVisible,
                               bool guidedWorkspace, bool viewDropdownOpen);

bool DrawEditorPanel(Rectangle bounds, const char* title);

bool DrawEditorToolbar(bool& workspaceVisible, bool& propertiesVisible,
                       Camera3D& camera, viewIndex& currentView,
                       viewIndex& lastView, bool& viewDropdownOpen,
                       bool& cameraLocked, bool guidedWorkspace);

#endif
