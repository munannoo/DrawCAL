#include "textures.h"
#include "raylib.h"
#include <iostream>

static Texture2D metalTexture = { 0 };
static bool textureLoaded = false;

void LoadObjectTextures(){
    if (textureLoaded) return;
    const char* path = "textures/Metal_texture.png";
    metalTexture = LoadTexture(path);
    if (metalTexture.id == 0)
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return;
    }
    textureLoaded = true;
}
void ApplyMetalTexture(Model& model){
    if (!textureLoaded || metalTexture.id == 0)
    {
        std::cerr << "Texture was not loaded before applying to model." << std::endl;
        return;
    }
    if (model.materialCount > 0)
    {
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = metalTexture;
    }
}
void UnloadTextures(){
    if (textureLoaded && metalTexture.id != 0)
    {
        UnloadTexture(metalTexture);
        metalTexture = { 0 };
        textureLoaded = false;
    }
}