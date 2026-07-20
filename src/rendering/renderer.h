#ifndef renderer_h
#define renderer_h

#include "raylib.h"
#include <r3d.h>

void initgridShader();
void DrawCameraScene(const Camera3D& camera, Rectangle viewport, RenderTexture2D& target);

// The object has no colour by default, if you wish to set colour for the object, you can do so by changing the material albedo
// Implement later:     material.albedo.color = newColor;

void renderLightObjects();
void renderAllObjects();


#endif // renderer_h
