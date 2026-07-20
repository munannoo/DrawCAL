#include "features/shadings/textures.h"

#include "raylib.h"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <future>

//PBRMaterial concreteMaterial = { 0 };
//PBRMaterial woodMaterial = { 0 };
//PBRMaterial plasticMaterial = { 0 };
//
//PBRMaterial cobblestoneMaterial = { 0 };
//
//PBRMaterial brickMaterial = { 0 };
//PBRMaterial tilesMaterial = { 0 };
//PBRMaterial metalMaterial = { 0 };
//PBRMaterial marbleMaterial = { 0 };
//PBRMaterial asphaltMaterial = { 0 };

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

Image PackORM(
    const char* aoPath,
    const char* roughnessPath,
    const char* metallicPath
)
{
    Image ao = LoadImage(aoPath);
    Image roughness = LoadImage(roughnessPath);
    Image metallic = LoadImage(metallicPath);

    // They must have matching dimensions
    if (ao.width != roughness.width ||
        ao.height != roughness.height ||
        ao.width != metallic.width ||
        ao.height != metallic.height)
    {
        TraceLog(LOG_ERROR, "ORM texture dimensions do not match");

        UnloadImage(ao);
        UnloadImage(roughness);
        UnloadImage(metallic);

        return {};
    }

    Color* aoPixels = LoadImageColors(ao);
    Color* roughnessPixels = LoadImageColors(roughness);
    Color* metallicPixels = LoadImageColors(metallic);

    int pixelCount = ao.width * ao.height;

    Color* ormPixels = static_cast<Color*>(MemAlloc(pixelCount * sizeof(Color)));

    for (int i = 0; i < pixelCount; i++)
    {
        ormPixels[i] = {
            aoPixels[i].r,          // R = Occlusion
            roughnessPixels[i].r,   // G = Roughness
            metallicPixels[i].r,    // B = Metalness
            255
        };
    }

    Image orm = {
        ormPixels,
        ao.width,
        ao.height,
        1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    UnloadImageColors(aoPixels);
    UnloadImageColors(roughnessPixels);
    UnloadImageColors(metallicPixels);

    UnloadImage(ao);
    UnloadImage(roughness);
    UnloadImage(metallic);

    return orm;
}

static bool texturesLoaded = false;

// Stores each unique GPU texture once.
// Materials can share Texture2D structs that point to the same OpenGL texture id.
struct CachedTexture
{
    std::string path;
    Texture2D texture;
};

static std::vector<CachedTexture> textureCache;

// Used to avoid rebinding the same material to the same Model material every draw.
static std::unordered_map<Material*, MaterialType> appliedMaterialCache;
struct TextureRequest
{
    std::string path;
    Color fallbackColor;
};

struct MaterialTextureSetPaths
{
    R3D_Material* target;

    const char* albedoPath;
    const char* normalPath;
    const char* ormPath;
    const char* metallicPath;
    const char* roughnessPath;
    const char* aoPath;
    const char* heightPath;
};

struct ImageLoadResult
{
    std::string path;
    Image image = { 0 };
    bool usedFallback = false;
    bool missingFile = false;
    bool decodeFailed = false;
};

static ImageLoadResult LoadImageSafeAsync(std::string path, Color fallbackColor)
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

static void AddTextureRequest(
    std::unordered_map<std::string, Color>& requests,
    const char* path,
    Color fallbackColor
)
{
    if (path == nullptr || path[0] == '\0') return;

    // If the same path appears many times, load it only once.
    if (requests.find(path) == requests.end())
    {
        requests[path] = fallbackColor;
    }
}

static Texture2D GetUploadedTexture(
    const std::unordered_map<std::string, Texture2D>& uploadedTextures,
    const char* path
)
{
    auto it = uploadedTextures.find(path);

    if (it != uploadedTextures.end())
    {
        return it->second;
    }

    return Texture2D{ 0 };
}

static void AssignMaterialTextures(
    R3D_Material& material,
    const MaterialTextureSetPaths& paths,
    const std::unordered_map<std::string, Texture2D>& uploadedTextures
)
{
    //material.albedo = GetUploadedTexture(uploadedTextures, paths.albedoPath);
    //material.normal = GetUploadedTexture(uploadedTextures, paths.normalPath);
    //material.metallic = GetUploadedTexture(uploadedTextures, paths.metallicPath);
    //material.roughness = GetUploadedTexture(uploadedTextures, paths.roughnessPath);
    //material.ao = GetUploadedTexture(uploadedTextures, paths.aoPath);
    //material.height = GetUploadedTexture(uploadedTextures, paths.heightPath);
    material.albedo.texture = GetUploadedTexture(uploadedTextures, paths.albedoPath);
    material.normal.texture = GetUploadedTexture(uploadedTextures, paths.normalPath);
    Image ormImage = PackORM( paths.aoPath, paths.roughnessPath, paths.metallicPath );
    material.orm.texture = LoadTextureFromImage(ormImage);
    UnloadImage(ormImage);    //material.roughness = GetUploadedTexture(uploadedTextures, paths.roughnessPath);
    //material.ao = GetUploadedTexture(uploadedTextures, paths.aoPath);
    //material.height = GetUploadedTexture(uploadedTextures, paths.heightPath);
}

void LoadPBRTextures()
{
    if (texturesLoaded) return;

    const double startTime = GetTime();

	// set to default values in case of unknown fields inside R3D_Material struct, so that we don't want to be 0
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

    MaterialTextureSetPaths materialSets[] =
    {
        {
            &woodMaterial,
            "textures/wood/WoodFloor_Color.png",
            "textures/wood/WoodFloor_Normal.png",
            "textures/default/black.png",
            "textures/wood/WoodFloor_Roughness.png",
            "textures/wood/WoodFloor_AO.png",
            "textures/wood/WoodFloor_Displacement.png"
        },
        {
            &cobblestoneMaterial,
            "textures/cobblestone/diff.png",
            "textures/cobblestone/normal.png",
            "textures/default/black.png",
            "textures/cobblestone/rough.png",
            "textures/cobblestone/ao.png",
            "textures/cobblestone/disp.png"
        },
        {
            &concreteMaterial,
            "textures/concrete/diff.png",
            "textures/concrete/normal.png",
            "textures/concrete/metal.png",
            "textures/concrete/rough.png",
            "textures/concrete/ao.png",
            "textures/concrete/disp.png"
        },
        {
            &plasticMaterial,
            "textures/plaster/diff.png",
            "textures/plaster/normal.png",
            "textures/plaster/metal.png",
            "textures/plaster/rough.png",
            "textures/plaster/ao.png",
            "textures/plaster/disp.png"
        },
        {
            &brickMaterial,
            "textures/brick/diff.png",
            "textures/brick/normal.png",
            "textures/brick/metal.png",
            "textures/brick/rough.png",
            "textures/brick/ao.png",
            "textures/brick/disp.png"
        },
        {
            &tilesMaterial,
            "textures/tiles/diff.png",
            "textures/tiles/normal.png",
            "textures/tiles/metal.png",
            "textures/tiles/rough.png",
            "textures/tiles/ao.png",
            "textures/tiles/disp.png"
        },
        {
            &metalMaterial,
            "textures/metal/diff.png",
            "textures/metal/normal.png",
            "textures/metal/metal.png",
            "textures/metal/rough.png",
            "textures/metal/ao.png",
            "textures/metal/disp.png"
        },
        {
            &marbleMaterial,
            "textures/marble/diff.png",
            "textures/marble/normal.png",
            "textures/marble/metal.png",
            "textures/marble/rough.png",
            "textures/marble/ao.png",
            "textures/marble/disp.png"
        },
        {
            &asphaltMaterial,
            "textures/asphalt/diff.png",
            "textures/asphalt/normal.png",
            "textures/asphalt/metal.png",
            "textures/asphalt/rough.png",
            "textures/asphalt/ao.png",
            "textures/asphalt/disp.png"
        }
    };

    std::unordered_map<std::string, Color> textureRequests;

    for (const MaterialTextureSetPaths& set : materialSets)
    {
        AddTextureRequest(textureRequests, set.albedoPath, WHITE);
        AddTextureRequest(textureRequests, set.normalPath, Color{ 128, 128, 255, 255 });
        AddTextureRequest(textureRequests, set.metallicPath, BLACK);
        AddTextureRequest(textureRequests, set.roughnessPath, WHITE);
        AddTextureRequest(textureRequests, set.aoPath, WHITE);
        AddTextureRequest(textureRequests, set.heightPath, BLACK);
    }

    std::vector<std::future<ImageLoadResult>> futures;
    futures.reserve(textureRequests.size());

    // Parallel CPU-side loading/decompression only.
    // Do NOT call LoadTexture() in these worker threads.
    for (const auto& request : textureRequests)
    {
        futures.push_back(
            std::async(
                std::launch::async,
                LoadImageSafeAsync,
                request.first,
                request.second
            )
        );
    }

    std::unordered_map<std::string, Texture2D> uploadedTextures;
    uploadedTextures.reserve(textureRequests.size());
    textureCache.reserve(textureRequests.size());

    // Main-thread GPU upload.
    for (std::future<ImageLoadResult>& future : futures)
    {
        ImageLoadResult result;

        try{
            result = future.get();
        }
        catch (const std::exception& e){
            std::cout << "Texture loading thread failed: " << e.what() << "\n";

            result.path = "async_failed_fallback";
            result.image = GenImageColor(1, 1, MAGENTA);
            result.usedFallback = true;
            result.decodeFailed = true;
        }

        if (result.usedFallback){
            if (result.missingFile){
                std::cout << "Missing texture: " << result.path << ". Using fallback.\n";
            }
            else if (result.decodeFailed){
                std::cout << "Failed to decode texture: " << result.path << ". Using fallback.\n";
            }
        }

        Texture2D texture = LoadTextureFromImage(result.image);
        UnloadImage(result.image);

        ConfigureTextureFor3D(texture, true);

        uploadedTextures[result.path] = texture;
        textureCache.push_back(CachedTexture{ result.path, texture });
    }

    for (const MaterialTextureSetPaths& set : materialSets)
    {
        AssignMaterialTextures(*set.target, set, uploadedTextures);
    }

    texturesLoaded = true;

    const double endTime = GetTime();

    TraceLog(
        LOG_INFO,
        "Loaded %i unique PBR textures in %.3f seconds",
        (int)textureCache.size(),
        endTime - startTime
    );
}

R3D_Material* GetMaterial(MaterialType type)
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
            return &concreteMaterial;
        default:
            return &defaultMaterial;
    }
}

//void ApplyPBRMaterial(Model& model, MaterialType type)
//{
//    if (model.materialCount <= 0) return;
//
//    Material& mat = model.materials[0];
//
//    // Avoid rebinding the same textures every single draw call.
//    auto it = appliedMaterialCache.find(&mat);
//
//    if (it != appliedMaterialCache.end() && it->second == type)
//    {
//        return;
//    }
//
//    PBRMaterial* matData = GetMaterial(type);
//
//    mat.maps[MATERIAL_MAP_ALBEDO].texture = matData->albedo;
//    mat.maps[MATERIAL_MAP_NORMAL].texture = matData->normal;
//    mat.maps[MATERIAL_MAP_METALNESS].texture = matData->metallic;
//    mat.maps[MATERIAL_MAP_ROUGHNESS].texture = matData->roughness;
//    mat.maps[MATERIAL_MAP_OCCLUSION].texture = matData->ao;
//    mat.maps[MATERIAL_MAP_HEIGHT].texture = matData->height;
//
//    mat.maps[MATERIAL_MAP_ALBEDO].color = WHITE;
//    mat.maps[MATERIAL_MAP_METALNESS].value = 0.0f;
//    mat.maps[MATERIAL_MAP_ROUGHNESS].value = 1.0f;
//    mat.maps[MATERIAL_MAP_OCCLUSION].value = 1.0f;
//
//    appliedMaterialCache[&mat] = type;
//}

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
    if (!texturesLoaded) return;

    // Only unload unique cached GPU textures.
    // Do NOT unload each PBRMaterial separately because many materials share the same Texture2D id.
    for (CachedTexture& cached : textureCache)
    {
        UnloadTextureSafe(cached.texture);
    }

    textureCache.clear();
    appliedMaterialCache.clear();

    woodMaterial = { 0 };
    concreteMaterial = { 0 };
    plasticMaterial = { 0 };
    cobblestoneMaterial = { 0 };
    brickMaterial = { 0 };
    tilesMaterial = { 0 };
    metalMaterial = { 0 };
    marbleMaterial = { 0 };
    asphaltMaterial = { 0 };

    texturesLoaded = false;
}