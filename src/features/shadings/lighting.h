#pragma once

#include "raylib.h"

void InitLighting();
void ApplyLightingShader(Model& model);
void UpdateLighting(Camera3D camera);
void UnloadLighting();