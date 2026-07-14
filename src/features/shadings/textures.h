#pragma once

#include "raylib.h"
#include <r3d.h>
#include <cstring>
// Because R3d uses ORM texture packing, we need to pack the occlusion, roughness, and metallic maps into a single texture.
Image PackORM(
    const char* aoPath,
    const char* roughnessPath,
    const char* metallicPath
);

//struct PBRMaterial
//{
//    Texture2D albedo;
//    Texture2D normal;
//    Texture2D metallic;
//    Texture2D roughness;
//    Texture2D ao;
//    Texture2D height;
//};

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

// Initialize static default material from R3D default
// Do this lazily: s_defaultMaterial must be populated once R3D is available.
// If R3D_GetDefaultMaterial is not ready yet, the static will be updated on first use.

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

//extern PBRMaterial concreteMaterial;
//extern PBRMaterial woodMaterial;
//extern PBRMaterial plasticMaterial;
//extern PBRMaterial cobblestoneMaterial;
//extern PBRMaterial brickMaterial;
//extern PBRMaterial tilesMaterial;
//extern PBRMaterial metalMaterial;
//extern PBRMaterial marbleMaterial;
//extern PBRMaterial asphaltMaterial;

void LoadPBRTextures();
void UnloadTextures();
R3D_Material* GetMaterial(MaterialType type);

void ConfigureTextureFor3D(Texture2D& texture, bool repeat);
void ApplyPBRMaterial(Model& model, MaterialType type);