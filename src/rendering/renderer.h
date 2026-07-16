#ifndef renderer_h
#define renderer_h

#include "raylib.h"

void initgridShader();
void DrawCameraScene(const Camera3D &camera, bool drawTransformGizmo = true);

#endif // renderer_h
