#include "features/shadings/textures.h"

#include "raylib.h"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map> // makes an unordered tab (key -> storage) for fast lookups

R3D_Material defaultMaterial{};
R3D_Material concreteMaterial{};
R3D_Material woodMaterial{};
R3D_Material plasticMaterial{};
R3D_Material cobblestoneMaterial{};
R3D_Material brickMaterial{};
R3D_Material tilesMaterial{};
R3D_Material metalMaterial{};
R3D_Material marbleMaterial{};
R3D_Material asphaltMaterial{};


struct ImageLoadResult
{
    std::string path;
    Image image = { 0 };
    bool usedFallback = false;
    bool missingFile = false;
    bool decodeFailed = false;
};

static ImageLoadResult LoadImageSafe(const std::string& path, Color fallbackColor)
{
    ImageLoadResult result;
    result.path = path;

    if (!FileExists(path.c_str()))
    {
        result.image = GenImageColor(1, 1, fallbackColor);
        result.usedFallback = true;
        result.missingFile = true;
        return result;
    }

    result.image = LoadImage(path.c_str());

    if (result.image.data == nullptr || result.image.width <= 0 || result.image.height <= 0)
    {
        result.image = GenImageColor(1, 1, fallbackColor);
        result.usedFallback = true;
        result.decodeFailed = true;
    }

    return result;
}


Image PackORM(
    const char* aoPath,
    const char* roughnessPath,
    const char* metallicPath
)
{

    ImageLoadResult aoResult = LoadImageSafe(aoPath, WHITE);            // AO defaults to "fully lit"
    ImageLoadResult roughnessResult = LoadImageSafe(roughnessPath, WHITE);            // roughness defaults to "fully rough"
    ImageLoadResult metallicResult = LoadImageSafe(metallicPath, BLACK);            // metallic defaults to "non-metal"

    Image ao = aoResult.image;
    Image roughness = roughnessResult.image;
    Image metallic = metallicResult.image;

    if (aoResult.usedFallback)
        std::cout << "Missing/invalid AO map: " << aoPath << ". Using fallback.\n";
    if (roughnessResult.usedFallback)
        std::cout << "Missing/invalid roughness map: " << roughnessPath << ". Using fallback.\n";
    if (metallicResult.usedFallback)
        std::cout << "Missing/invalid metallic map: " << metallicPath << ". Using fallback.\n";

    // If any map used a 1x1 fallback while the others are full-size, resize the
    // fallback(s) up rather than failing the whole ORM pack over one missing file.
    int targetWidth = std::max({ ao.width,  roughness.width,  metallic.width });
    int targetHeight = std::max({ ao.height, roughness.height, metallic.height });

    if (ao.width != targetWidth || ao.height != targetHeight)
        ImageResize(&ao, targetWidth, targetHeight);
    if (roughness.width != targetWidth || roughness.height != targetHeight)
        ImageResize(&roughness, targetWidth, targetHeight);
    if (metallic.width != targetWidth || metallic.height != targetHeight)
        ImageResize(&metallic, targetWidth, targetHeight);

    Color* aoPixels = LoadImageColors(ao);
    Color* roughnessPixels = LoadImageColors(roughness);
    Color* metallicPixels = LoadImageColors(metallic);

    int pixelCount = ao.width * ao.height;

    Color* ormPixels = static_cast<Color*>(MemAlloc(pixelCount * sizeof(Color)));

    for (int i = 0; i < pixelCount; i++)
    {
        ormPixels[i] = { aoPixels[i].r, roughnessPixels[i].r, metallicPixels[i].r, 255 };
    }

    Image orm = { ormPixels, ao.width, ao.height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

    UnloadImageColors(aoPixels);
    UnloadImageColors(roughnessPixels);
    UnloadImageColors(metallicPixels);

    UnloadImage(ao);
    UnloadImage(roughness);
    UnloadImage(metallic);

    return orm;
}

struct CachedTexture
{
    std::string key;
    Texture2D texture;
};

static std::vector<CachedTexture> textureCache;
static std::unordered_map<std::string, Texture2D> uploadedTextures;

struct MaterialTextureSetPaths
{
    R3D_Material* target;

    const char* albedoPath;
    const char* normalPath;
    const char* ormPath; // unused for now, kept for parity with original struct
    const char* metallicPath;
    const char* roughnessPath;
    const char* aoPath;
    const char* heightPath;
};

void ConfigureTextureFor3D(Texture2D& texture, bool repeat)
{
    if (texture.id == 0) return;

    GenTextureMipmaps(&texture);
    SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);

    if (repeat)
        SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
    else
        SetTextureWrap(texture, TEXTURE_WRAP_CLAMP);
}

// Loads (or reuses from cache) a single texture by path
static Texture2D GetOrLoadTexture(const char* path, Color fallbackColor)
{
    if (path == nullptr || path[0] == '\0')
        return Texture2D{ 0 };

    std::string key = path;

    {
        auto it = uploadedTextures.find(key);
        if (it != uploadedTextures.end())
            return it->second;
    }

    ImageLoadResult result = LoadImageSafe(key, fallbackColor);

    if (result.usedFallback)
    {
        if (result.missingFile)
            std::cout << "Missing texture: " << result.path << ". Using fallback.\n";
        else if (result.decodeFailed)
            std::cout << "Failed to decode texture: " << result.path << ". Using fallback.\n";
    }

    Texture2D texture = LoadTextureFromImage(result.image);
    UnloadImage(result.image);
    ConfigureTextureFor3D(texture, true);

    uploadedTextures[key] = texture;
    textureCache.push_back(CachedTexture{ key, texture });

    return texture;
}

// Loads (or reuses) the packed ORM texture for a given ao/roughness/metallic triple.
static Texture2D GetOrLoadORMTexture(const char* aoPath, const char* roughnessPath, const char* metallicPath)
{
    std::string key = std::string(aoPath) + "|" + roughnessPath + "|" + metallicPath;

    {
        auto it = uploadedTextures.find(key);
        if (it != uploadedTextures.end())
            return it->second;
    }

    Image orm = PackORM(aoPath, roughnessPath, metallicPath);

    if (orm.data == nullptr || orm.width <= 0 || orm.height <= 0)
    {
        TraceLog(LOG_ERROR, "Failed to pack ORM texture for %s", key.c_str());
        UnloadImage(orm);
        return Texture2D{ 0 };
    }

    Texture2D texture = LoadTextureFromImage(orm);
    UnloadImage(orm);
    ConfigureTextureFor3D(texture, true);

    uploadedTextures[key] = texture;
    textureCache.push_back(CachedTexture{ key, texture });

    return texture;
}


struct MaterialEntry
{
    MaterialTextureSetPaths paths;
    bool loaded = false;
};

static std::unordered_map<int, MaterialEntry> materialTable;
static bool materialTableBuilt = false;
static bool defaultMaterialInitialized = false;

static void EnsureDefaultMaterial()
{
    if (defaultMaterialInitialized) return;

    defaultMaterial = R3D_GetDefaultMaterial();

    concreteMaterial = defaultMaterial;
    woodMaterial = defaultMaterial;
    plasticMaterial = defaultMaterial;
    cobblestoneMaterial = defaultMaterial;
    brickMaterial = defaultMaterial;
    tilesMaterial = defaultMaterial;
    metalMaterial = defaultMaterial;
    marbleMaterial = defaultMaterial;
    asphaltMaterial = defaultMaterial;

    defaultMaterialInitialized = true;
}

static void BuildMaterialTable()
{
    if (materialTableBuilt) return;

    materialTable[MATERIAL_WOOD] = MaterialEntry{ {
        &woodMaterial,
        "textures/wood/WoodFloor_Color.png",
        "textures/wood/WoodFloor_Normal.png",
        "textures/default/black.png",
        "textures/wood/WoodFloor_Roughness.png",
        "textures/wood/WoodFloor_AO.png",
        "textures/wood/WoodFloor_Displacement.png"
    } };

    materialTable[MATERIAL_COBBLESTONE] = MaterialEntry{ {
        &cobblestoneMaterial,
        "textures/cobblestone/diff.png",
        "textures/cobblestone/normal.png",
        "textures/default/black.png",
        "textures/cobblestone/rough.png",
        "textures/cobblestone/ao.png",
        "textures/cobblestone/disp.png"
    } };

    materialTable[MATERIAL_CONCRETE] = MaterialEntry{ {
        &concreteMaterial,
        "textures/concrete/diff.png",
        "textures/concrete/normal.png",
        "textures/concrete/metal.png",
        "textures/concrete/rough.png",
        "textures/concrete/ao.png",
        "textures/concrete/disp.png"
    } };

    materialTable[MATERIAL_PLASTIC] = MaterialEntry{ {
        &plasticMaterial,
        "textures/plaster/diff.png",
        "textures/plaster/normal.png",
        "textures/plaster/metal.png",
        "textures/plaster/rough.png",
        "textures/plaster/ao.png",
        "textures/plaster/disp.png"
    } };

    materialTable[MATERIAL_BRICK] = MaterialEntry{ {
        &brickMaterial,
        "textures/brick/diff.png",
        "textures/brick/normal.png",
        "textures/brick/metal.png",
        "textures/brick/rough.png",
        "textures/brick/ao.png",
        "textures/brick/disp.png"
    } };

    materialTable[MATERIAL_TILES] = MaterialEntry{ {
        &tilesMaterial,
        "textures/tiles/diff.png",
        "textures/tiles/normal.png",
        "textures/tiles/metal.png",
        "textures/tiles/rough.png",
        "textures/tiles/ao.png",
        "textures/tiles/disp.png"
    } };

    materialTable[MATERIAL_METAL] = MaterialEntry{ {
        &metalMaterial,
        "textures/metal/diff.png",
        "textures/metal/normal.png",
        "textures/metal/metal.png",
        "textures/metal/rough.png",
        "textures/metal/ao.png",
        "textures/metal/disp.png"
    } };

    materialTable[MATERIAL_MARBLE] = MaterialEntry{ {
        &marbleMaterial,
        "textures/marble/diff.png",
        "textures/marble/normal.png",
        "textures/marble/metal.png",
        "textures/marble/rough.png",
        "textures/marble/ao.png",
        "textures/marble/disp.png"
    } };

    materialTable[MATERIAL_ASPHALT] = MaterialEntry{ {
        &asphaltMaterial,
        "textures/asphalt/diff.png",
        "textures/asphalt/normal.png",
        "textures/asphalt/metal.png",
        "textures/asphalt/rough.png",
        "textures/asphalt/ao.png",
        "textures/asphalt/disp.png"
    } };

    materialTableBuilt = true;
}

void LoadPBRTextures()
{
    // lightweight here. Materials get their real textures the first time GetMaterial() is called for their type.
    EnsureDefaultMaterial();
    BuildMaterialTable();

    TraceLog(LOG_INFO, "PBR material table ready (%i materials registered for lazy load)", (int)materialTable.size());
}

static void LoadMaterialTextures(MaterialEntry& entry)
{
    const double startTime = GetTime();

    entry.paths.target->albedo.texture = GetOrLoadTexture(entry.paths.albedoPath, WHITE);
    entry.paths.target->normal.texture = GetOrLoadTexture(entry.paths.normalPath, Color{ 128, 128, 255, 255 });
    entry.paths.target->orm.texture = GetOrLoadORMTexture(entry.paths.aoPath, entry.paths.roughnessPath, entry.paths.metallicPath);

    entry.loaded = true;

    TraceLog(LOG_INFO, "Lazily loaded material textures in %.3f seconds", GetTime() - startTime);
}

R3D_Material* GetMaterial(MaterialType type)
{
    EnsureDefaultMaterial();

    if (type == MATERIAL_NONE)
        return &defaultMaterial;

    BuildMaterialTable();

    auto it = materialTable.find(type);
    if (it == materialTable.end())
        return &defaultMaterial;

    if (!it->second.loaded)
    {
        LoadMaterialTextures(it->second);
    }

    return it->second.paths.target;
}

void PreloadMaterial(MaterialType type)
{
    // Just forces the same lazy path to run now instead of on next use.
    GetMaterial(type);
}

void PreloadAllMaterials()
{
    BuildMaterialTable();
    for (auto& [type, entry] : materialTable)
    {
        if (!entry.loaded)
            LoadMaterialTextures(entry);
    }
}

bool IsMaterialLoaded(MaterialType type)
{
    auto it = materialTable.find(type);
    return it != materialTable.end() && it->second.loaded;
}

static void UnloadTextureSafe(Texture2D& texture)
{
    if (texture.id != 0)
    {
        UnloadTexture(texture);
        texture = { 0 };
    }
}

void UnloadTextures()
{
    for (CachedTexture& cached : textureCache)
    {
        UnloadTextureSafe(cached.texture);
    }

    textureCache.clear();
    uploadedTextures.clear();

    for (auto& [type, entry] : materialTable)
    {
        entry.loaded = false;
    }

    woodMaterial = { 0 };
    concreteMaterial = { 0 };
    plasticMaterial = { 0 };
    cobblestoneMaterial = { 0 };
    brickMaterial = { 0 };
    tilesMaterial = { 0 };
    metalMaterial = { 0 };
    marbleMaterial = { 0 };
    asphaltMaterial = { 0 };

    defaultMaterialInitialized = false;
}