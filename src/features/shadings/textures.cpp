#include "features/shadings/textures.h"

#include "raylib.h"
#include <iostream>

PBRMaterial concreteMaterial = { 0 };
PBRMaterial woodMaterial = { 0 };
PBRMaterial plasticMaterial = { 0 };

PBRMaterial cobblestoneMaterial = { 0 };

PBRMaterial brickMaterial = { 0 };
PBRMaterial tilesMaterial = { 0 };
PBRMaterial metalMaterial = { 0 };
PBRMaterial marbleMaterial = { 0 };
PBRMaterial asphaltMaterial = { 0 };

static bool texturesLoaded = false;

static Texture2D LoadTextureSafe(const char* path, Color fallbackColor)
{
    Texture2D tex = LoadTexture(path);

    if (tex.id == 0)
    {
        std::cout << "Failed to load texture: " << path << ". Using fallback.\n";

        Image img = GenImageColor(1, 1, fallbackColor);
        tex = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    return tex;
}

void ConfigureTextureFor3D(Texture2D& texture, bool repeat)
{
    if (texture.id == 0) return;

    GenTextureMipmaps(&texture);
    SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);

    if (repeat)
    {
        SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
    }
    else
    {
        SetTextureWrap(texture, TEXTURE_WRAP_CLAMP);
    }
}

static PBRMaterial LoadMaterialTextureSet(
    const char* albedoPath,
    const char* normalPath,
    const char* metallicPath,
    const char* roughnessPath,
    const char* aoPath,
    const char* heightPath
)
{
    PBRMaterial material = { 0 };

    material.albedo = LoadTextureSafe(albedoPath, WHITE);
    ConfigureTextureFor3D(material.albedo, true);

    material.normal = LoadTextureSafe(normalPath, Color{ 128, 128, 255, 255 });
    ConfigureTextureFor3D(material.normal, true);

    material.metallic = LoadTextureSafe(metallicPath, BLACK);
    ConfigureTextureFor3D(material.metallic, true);

    material.roughness = LoadTextureSafe(roughnessPath, WHITE);
    ConfigureTextureFor3D(material.roughness, true);

    material.ao = LoadTextureSafe(aoPath, WHITE);
    ConfigureTextureFor3D(material.ao, true);

    material.height = LoadTextureSafe(heightPath, BLACK);
    ConfigureTextureFor3D(material.height, true);

    return material;
}

void LoadPBRTextures()
{
    if (texturesLoaded) return;

    woodMaterial = LoadMaterialTextureSet(
        "textures/wood/WoodFloor_Color.png",
        "textures/wood/WoodFloor_Normal.png",
        "textures/default/black.png",
        "textures/wood/WoodFloor_Roughness.png",
        "textures/wood/WoodFloor_AO.png",
        "textures/wood/WoodFloor_Displacement.png"
    );

    cobblestoneMaterial = LoadMaterialTextureSet(
        "textures/cobblestone/diff.png",
        "textures/cobblestone/normal.png",
        "textures/default/black.png",
        "textures/cobblestone/rough.png",
        "textures/cobblestone/ao.png",
        "textures/cobblestone/disp.png"
    );

    concreteMaterial = LoadMaterialTextureSet(
        "textures/concrete/diff.png",
        "textures/concrete/normal.png",
        "textures/concrete/metal.png",
        "textures/concrete/rough.png",
        "textures/concrete/ao.png",
        "textures/concrete/disp.png"
    );

    plasticMaterial = LoadMaterialTextureSet(
        "textures/plaster/diff.png",
        "textures/plaster/normal.png",
        "textures/plaster/metal.png",
        "textures/plaster/rough.png",
        "textures/plaster/ao.png",
        "textures/plaster/disp.png"
    );

    brickMaterial = LoadMaterialTextureSet(
        "textures/brick/diff.png",
        "textures/brick/normal.png",
        "textures/brick/metal.png",
        "textures/brick/rough.png",
        "textures/brick/ao.png",
        "textures/brick/disp.png"
    );

    tilesMaterial = LoadMaterialTextureSet(
        "textures/tiles/diff.png",
        "textures/tiles/normal.png",
        "textures/tiles/metal.png",
        "textures/tiles/rough.png",
        "textures/tiles/ao.png",
        "textures/tiles/disp.png"
    );

    metalMaterial = LoadMaterialTextureSet(
        "textures/metal/diff.png",
        "textures/metal/normal.png",
        "textures/metal/metal.png",
        "textures/metal/rough.png",
        "textures/metal/ao.png",
        "textures/metal/disp.png"
    );

    marbleMaterial = LoadMaterialTextureSet(
        "textures/marble/diff.png",
        "textures/marble/normal.png",
        "textures/marble/metal.png",
        "textures/marble/rough.png",
        "textures/marble/ao.png",
        "textures/marble/disp.png"
    );

    asphaltMaterial = LoadMaterialTextureSet(
        "textures/asphalt/diff.png",
        "textures/asphalt/normal.png",
        "textures/asphalt/metal.png",
        "textures/asphalt/rough.png",
        "textures/asphalt/ao.png",
        "textures/asphalt/disp.png"
    );

    texturesLoaded = true;
}

static PBRMaterial* GetMaterial(MaterialType type)
{
    switch (type)
    {
        case MATERIAL_WOOD:
            return &woodMaterial;

        case MATERIAL_PLASTIC:
            return &plasticMaterial;

        case MATERIAL_COBBLESTONE:
            return &cobblestoneMaterial;

        case MATERIAL_BRICK:
            return &brickMaterial;

        case MATERIAL_TILES:
            return &tilesMaterial;

        case MATERIAL_METAL:
            return &metalMaterial;

        case MATERIAL_MARBLE:
            return &marbleMaterial;

        case MATERIAL_ASPHALT:
            return &asphaltMaterial;

        case MATERIAL_CONCRETE:
        default:
            return &concreteMaterial;
    }
}

void ApplyPBRMaterial(Model& model, MaterialType type)
{
    if (model.materialCount <= 0) return;

    PBRMaterial* matData = GetMaterial(type);
    Material& mat = model.materials[0];

    mat.maps[MATERIAL_MAP_ALBEDO].texture = matData->albedo;
    mat.maps[MATERIAL_MAP_NORMAL].texture = matData->normal;
    mat.maps[MATERIAL_MAP_METALNESS].texture = matData->metallic;
    mat.maps[MATERIAL_MAP_ROUGHNESS].texture = matData->roughness;
    mat.maps[MATERIAL_MAP_OCCLUSION].texture = matData->ao;
    mat.maps[MATERIAL_MAP_HEIGHT].texture = matData->height;

    mat.maps[MATERIAL_MAP_ALBEDO].color = WHITE;
    mat.maps[MATERIAL_MAP_METALNESS].value = 0.0f;
    mat.maps[MATERIAL_MAP_ROUGHNESS].value = 1.0f;
    mat.maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
}

static void UnloadTextureSafe(Texture2D& texture)
{
    if (texture.id != 0)
    {
        UnloadTexture(texture);
        texture = { 0 };
    }
}

static void UnloadMaterialTextureSet(PBRMaterial& material)
{
    UnloadTextureSafe(material.albedo);
    UnloadTextureSafe(material.normal);
    UnloadTextureSafe(material.metallic);
    UnloadTextureSafe(material.roughness);
    UnloadTextureSafe(material.ao);
    UnloadTextureSafe(material.height);
}

void UnloadTextures()
{
    if (!texturesLoaded) return;

    UnloadMaterialTextureSet(woodMaterial);
    UnloadMaterialTextureSet(concreteMaterial);
    UnloadMaterialTextureSet(plasticMaterial);

    UnloadMaterialTextureSet(cobblestoneMaterial);

    UnloadMaterialTextureSet(brickMaterial);
    UnloadMaterialTextureSet(tilesMaterial);
    UnloadMaterialTextureSet(metalMaterial);
    UnloadMaterialTextureSet(marbleMaterial);
    UnloadMaterialTextureSet(asphaltMaterial);

    texturesLoaded = false;
}