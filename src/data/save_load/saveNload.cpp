#include "data/save_load/saveNload.h"

// since json is not natively supported by cpp
using json = nlohmann::json;

// Convert the raylib properties into json
static json vector3ToJson(Vector3 v) {
    return {
        {"x", v.x},
        {"y", v.y},
        {"z", v.z}
    };
}
static json colorToJson(Color color) {
    return {
        {"r", color.r},
        {"g", color.g},
        {"b", color.b},
        {"a", color.a}
    };
}
static json quaternionToJson(Quaternion q)
{
    return {
        {"x", q.x},
        {"y", q.y},
        {"z", q.z},
        {"w", q.w}
    };
}
static json transformToJson(const Transform& transform)
{
    return {
        {"translation", vector3ToJson(transform.translation)},
        {"rotation", quaternionToJson(transform.rotation)},
        {"scale", vector3ToJson(transform.scale)}
    };
}
static json meshDataToJson(const R3D_MeshData& meshData)
{
    json data;
    data["vertices"] = json::array();
    data["indices"] = json::array();

    for (int i = 0; i < meshData.vertexCount; i++)
    {
        const R3D_Vertex& vertex = meshData.vertices[i];

        data["vertices"].push_back({
            {"position", vector3ToJson(vertex.position)},
            {"texcoord", {vertex.texcoord[0], vertex.texcoord[1]}},
            {"normal",   {vertex.normal[0], vertex.normal[1], vertex.normal[2], vertex.normal[3]}},
            {"tangent",  {vertex.tangent[0], vertex.tangent[1], vertex.tangent[2], vertex.tangent[3]}},
            {"color",    {vertex.color.r, vertex.color.g, vertex.color.b, vertex.color.a}},
            {"boneIndices", {vertex.boneIndices[0], vertex.boneIndices[1], vertex.boneIndices[2], vertex.boneIndices[3]}},
            {"boneWeights", {vertex.boneWeights[0], vertex.boneWeights[1], vertex.boneWeights[2], vertex.boneWeights[3]}},
            });
    }

    for (int i = 0; i < meshData.indexCount; i++)
        data["indices"].push_back(meshData.indices[i]);

    return data;
}

static const char* MaterialTypeToString(MaterialType material)
{
    switch (material) {
    case MATERIAL_CONCRETE: return "concrete";
    case MATERIAL_WOOD: return "wood";
    case MATERIAL_PLASTIC: return "plastic";
    case MATERIAL_COBBLESTONE: return "cobblestone";
    case MATERIAL_BRICK: return "brick";
    case MATERIAL_TILES: return "tiles";
    case MATERIAL_METAL: return "metal";
    case MATERIAL_MARBLE: return "marble";
    case MATERIAL_ASPHALT: return "asphalt";
    case MATERIAL_NONE:
    default:
        return "default";
    }
}

static MaterialType StringToMaterialType(const std::string& material)
{
    if (material == "concrete") return MATERIAL_CONCRETE;
    if (material == "wood") return MATERIAL_WOOD;
    if (material == "plastic") return MATERIAL_PLASTIC;
    if (material == "cobblestone") return MATERIAL_COBBLESTONE;
    if (material == "brick") return MATERIAL_BRICK;
    if (material == "tiles") return MATERIAL_TILES;
    if (material == "metal") return MATERIAL_METAL;
    if (material == "marble") return MATERIAL_MARBLE;
    if (material == "asphalt") return MATERIAL_ASPHALT;

    return MATERIAL_NONE;
}

static const char* ObjectTypeToString(ObjectType type)
{
    switch (type) {
    case ObjectType::CUBE: return "cube";
    case ObjectType::SPHERE: return "sphere";
    case ObjectType::CYLINDER: return "cylinder";
    case ObjectType::CUSTOM: return "custom";
    default: return "cube";
    }
}

// Add new object information to the end of the json file
static void addObject(json& sceneData, shape& obj) {
    json objectData;
    objectData["type"] = ObjectTypeToString(obj.getObjectType()); // was: obj.getObjectType() (raw enum int, mismatched loadScene's string read)
    objectData["material"] = MaterialTypeToString(obj.getMaterialType());
    //objectData["color"] = colorToJson(obj.color);
    objectData["transform"] = transformToJson(obj.getTransform());
    //objectData["position"] = vector3ToJson(obj.getTransform().translation);
    //objectData["rotation"] = vector3ToJson(QuaternionToEuler(obj.getTransform().rotation));
    //objectData["scale"] = vector3ToJson(obj.getTransform().scale);
    objectData["meshData"] = meshDataToJson(obj.getMeshData());
    //objectData["isLight"] = obj.isLight;
    //objectData["lightIntensity"] = obj.lightIntensity;
    //objectData["lightRadius"] = obj.lightRadius;

    sceneData["objects"].push_back(objectData);
}


// Main Save Function, calls the other functions upwards
bool saveScene(const std::string& path) {
    std::ofstream sceneFile(path);

    if (!sceneFile.is_open()) {
        TraceLog(LOG_ERROR, "Could not open %s for saving.", path.c_str());
        return false;
    }

    json sceneData;

    sceneData["version"] = 2;
    sceneData["objects"] = json::array();

    for (const auto& object : objects)
    {
        if (object)
        {
            addObject(sceneData, *object);
        }
    }

    sceneFile << sceneData.dump(4);

    if (!sceneFile.good())
    {
        // NOTE: added — dump() succeeded but the stream could still fail on flush/write
        // (e.g. disk full, permissions revoked mid-write). Previously this was unchecked.
        TraceLog(LOG_ERROR, "Failed while writing scene data to %s.", path.c_str());
        return false;
    }

    TraceLog(LOG_INFO, "Scene saved to %s.", path.c_str());

    return true;
}

static Vector3 jsonToVector3(const json& data) {
    return Vector3{
        data.value("x", 0.0f),
        data.value("y", 0.0f),
        data.value("z", 0.0f)
    };
}
static Color jsonToColor(const json& data) {
    return Color{
        (unsigned char)data.value("r", 255),
        (unsigned char)data.value("g", 255),
        (unsigned char)data.value("b", 255),
        (unsigned char)data.value("a", 255)
    };
}
static Quaternion jsonToQuaternion(const json& data)
{
    return {
        data.value("x", 0.0f),
        data.value("y", 0.0f),
        data.value("z", 0.0f),
        data.value("w", 1.0f)
    };
}
static Transform jsonToTransform(const json& data)
{
    Transform transform{};

    transform.translation = jsonToVector3(data["translation"]);
    transform.rotation = jsonToQuaternion(data["rotation"]);
    transform.scale = jsonToVector3(data["scale"]);

    return transform;
}
static R3D_MeshData jsonToMeshData(const json& data)
{
    R3D_MeshData meshData{};

    meshData.vertexCount = static_cast<int>(data["vertices"].size());
    meshData.indexCount = static_cast<int>(data["indices"].size());

    meshData.vertices = new R3D_Vertex[meshData.vertexCount]{};
    meshData.indices = new uint32_t[meshData.indexCount]{};

    for (int i = 0; i < meshData.vertexCount; i++)
    {
        const json& vertexData = data["vertices"][i];
        R3D_Vertex& vertex = meshData.vertices[i];

        vertex.position = jsonToVector3(vertexData["position"]);
        vertex.texcoord[0] = vertexData["texcoord"][0].get<uint16_t>();
        vertex.texcoord[1] = vertexData["texcoord"][1].get<uint16_t>();

        if (vertexData.contains("normal"))
            for (int k = 0; k < 4; k++) vertex.normal[k] = vertexData["normal"][k].get<int8_t>();

        if (vertexData.contains("tangent"))
            for (int k = 0; k < 4; k++) vertex.tangent[k] = vertexData["tangent"][k].get<int8_t>();

        if (vertexData.contains("color"))
        {
            vertex.color.r = vertexData["color"][0].get<uint8_t>();
            vertex.color.g = vertexData["color"][1].get<uint8_t>();
            vertex.color.b = vertexData["color"][2].get<uint8_t>();
            vertex.color.a = vertexData["color"][3].get<uint8_t>();
        }
        else
        {
            vertex.color = WHITE; // fallback for old save files without color
        }

        if (vertexData.contains("boneIndices"))
            for (int k = 0; k < 4; k++) vertex.boneIndices[k] = vertexData["boneIndices"][k].get<uint8_t>();

        if (vertexData.contains("boneWeights"))
            for (int k = 0; k < 4; k++) vertex.boneWeights[k] = vertexData["boneWeights"][k].get<uint8_t>();
    }

    for (int i = 0; i < meshData.indexCount; i++)
        meshData.indices[i] = data["indices"][i].get<uint32_t>();

    return meshData;
}

static void loadObject(shape& obj, const json& objectData)
{
    obj.setMeshData(jsonToMeshData(objectData["meshData"]));

    MaterialType type = StringToMaterialType(
        objectData.value("material", "default")
    );

    obj.setMaterialType(type);

    obj.setTransform(jsonToTransform(objectData["transform"]));
}

bool loadScene(const std::string& path) {
    std::ifstream sceneFile(path);

    if (!sceneFile.is_open()) {
        TraceLog(LOG_ERROR, "%s not found.", path.c_str());
        return false;
    }

    json sceneData;
    try {
        sceneFile >> sceneData;
    }
    catch (const json::exception& error) {
        TraceLog(LOG_ERROR, "Could not load %s: %s", path.c_str(), error.what());

        return false;
    }

    if (!sceneData.contains("objects") || !sceneData["objects"].is_array())
    {
        TraceLog(LOG_ERROR, "Invalid scene file: %s", path.c_str());
        return false;
    }

    // Remove old selection references before destroying objects
    selectedObjects.clear();

    // Destroy all existing scene objects
    objects.clear();

    try
    {
        for (const auto& objectData : sceneData["objects"])
        {
            std::string type = objectData.value("type", "");
            std::unique_ptr<shape> object;

            if (type == "cube")
            {
                object = std::make_unique<cube>();
            }
            else if (type == "sphere")
            {
                object = std::make_unique<sphere>();
            }
            else if (type == "cylinder")
            {
                object = std::make_unique<cylinder>();
            }
            else
            {
                TraceLog(LOG_WARNING, "Unknown object type: %s", type.c_str());

                continue;
            }

            loadObject(*object, objectData);

            objects.push_back(std::move(object));
        }
    }
    catch (const json::exception& error)
    {
        TraceLog(LOG_ERROR, "Failed while loading objects from %s: %s", path.c_str(), error.what()
        );

        return false;
    }

    TraceLog(LOG_INFO, "Scene loaded from %s. Object count: %i", path.c_str(), static_cast<int>(objects.size())
    );

    return true;
}