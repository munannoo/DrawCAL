#pragma once

#include "raylib.h"
#include <r3d.h>
#include <cstring>

// Because R3D uses ORM texture packing, we need to pack the occlusion, roughness, and metallic maps into a single texture.
Image PackORM(
    const char* aoPath,
    const char* roughnessPath,
    const char* metallicPath
);

enum MaterialType
{
    MATERIAL_NONE = 0,
    MATERIAL_CONCRETE,
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

extern R3D_Material defaultMaterial;
extern R3D_Material concreteMaterial;
extern R3D_Material woodMaterial;
extern R3D_Material plasticMaterial;
extern R3D_Material cobblestoneMaterial;
extern R3D_Material brickMaterial;
extern R3D_Material tilesMaterial;
extern R3D_Material metalMaterial;
extern R3D_Material marbleMaterial;
extern R3D_Material asphaltMaterial;

// Lightweight setup only: builds the path table and default material.
// No texture files are touched here anymore.
void LoadPBRTextures();

void UnloadTextures();

// Returns the material for `type`, lazily loading its textures on first call.
// Must be called from the main/render thread (it may upload GPU textures).
R3D_Material* GetMaterial(MaterialType type);

// Optional: force a material to load immediately instead of waiting for first use
// (e.g. during a loading screen, to avoid a hitch mid-gameplay).
void PreloadMaterial(MaterialType type);
void PreloadAllMaterials();

bool IsMaterialLoaded(MaterialType type);

void ConfigureTextureFor3D(Texture2D& texture, bool repeat);
void ApplyPBRMaterial(Model& model, MaterialType type);