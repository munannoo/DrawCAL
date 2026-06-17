#pragma once

#include "raylib.h"

struct PBRMaterial
{
    Texture2D albedo;
    Texture2D normal;
    Texture2D metallic;
    Texture2D roughness;
    Texture2D ao;
    Texture2D height;
};

enum MaterialType
{
    MATERIAL_CONCRETE = 0,
    MATERIAL_WOOD,
    MATERIAL_PLASTIC,

    MATERIAL_COBBLESTONE,

    // Future material slots
    MATERIAL_BRICK,
    MATERIAL_TILES,
    MATERIAL_METAL,
    MATERIAL_MARBLE,
    MATERIAL_ASPHALT
};

extern PBRMaterial concreteMaterial;
extern PBRMaterial woodMaterial;
extern PBRMaterial plasticMaterial;
extern PBRMaterial cobblestoneMaterial;
extern PBRMaterial brickMaterial;
extern PBRMaterial tilesMaterial;
extern PBRMaterial metalMaterial;
extern PBRMaterial marbleMaterial;
extern PBRMaterial asphaltMaterial;

void LoadPBRTextures();
void UnloadTextures();

void ConfigureTextureFor3D(Texture2D& texture, bool repeat);
void ApplyPBRMaterial(Model& model, MaterialType type);