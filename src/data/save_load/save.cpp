#include "features/manipulation/Transform.h"
#include <iostream>
#include <fstream>
#include "json.hpp"
#include "objects/object.h"
#include "data/save_load/saveNload.h"
#include "features/shadings/lighting.h"
using json = nlohmann::json;

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

static void addObject(json& sceneData, const char* type, ObjectInstance& obj) {
    json objectData;

    objectData["type"] = type;
    objectData["color"] = colorToJson(obj.color);
    objectData["position"] = vector3ToJson(obj.position);
    objectData["rotation"] = vector3ToJson(obj.rotation);
    objectData["scale"] = vector3ToJson(obj.scale);

    objectData["isSelected"] = false;
    objectData["isLight"] = obj.isLight;

    sceneData["objects"].push_back(objectData);
}

void saveScene(
    ObjectInstance Cu[], int c,
    ObjectInstance Sp[], int s,
    ObjectInstance cy[], int y
) {
    std::ofstream sceneFile("scene.drawcal");

    if (!sceneFile.is_open()) {
        std::cout << "Could not open scene.drawcal for saving.\n";
        return;
    }

    json sceneData;
    sceneData["version"] = 1;
    sceneData["objects"] = json::array();

    for (int i = 0; i < c; i++) {
        addObject(sceneData, "cube", Cu[i]);
    }

    for (int i = 0; i < s; i++) {
        addObject(sceneData, "sphere", Sp[i]);
    }

    for (int i = 0; i < y; i++) {
        addObject(sceneData, "cylinder", cy[i]);
    }

    sceneFile << sceneData.dump(4);

    std::cout << "Scene saved.\n";
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

static void loadObject(ObjectInstance& obj, const json& objectData) {
    obj.color = jsonToColor(objectData["color"]);

    if (objectData.contains("position")) {
        obj.position = jsonToVector3(objectData["position"]);
    }
    else if (objectData.contains("coordinates")) {
        obj.position = jsonToVector3(objectData["coordinates"]);
    }

    obj.rotation = jsonToVector3(objectData["rotation"]);
    obj.scale = jsonToVector3(objectData["scale"]);

    obj.isSelected = false;
    obj.isLight = objectData.value("isLight", false);
    obj.lightIndex = -1;
}

bool loadScene(
    ObjectInstance Cu[], int& c,
    ObjectInstance Sp[], int& s,
    ObjectInstance cy[], int& y
) {
    std::ifstream sceneFile("scene.drawcal");

    if (!sceneFile.is_open()) {
        std::cout << "scene.drawcal not found. Creating default scene.\n";

        c = 1;
        s = 0;
        y = 0;

        Cu[0].position = Vector3{ 0.0f, 0.0f, 0.0f };
        Cu[0].rotation = Vector3{ 0.0f, 0.0f, 0.0f };
        Cu[0].scale = Vector3{ 1.0f, 1.0f, 1.0f };
        Cu[0].color = Color{ 130, 130, 130, 255 };
        Cu[0].isSelected = false;
        Cu[0].isLight = false;
        Cu[0].lightIndex = -1;

        saveScene(Cu, c, Sp, s, cy, y);

        return true;
    }

    json sceneData;
    sceneFile >> sceneData;

    c = 0;
    s = 0;
    y = 0;

    if (!sceneData.contains("objects")) {
        std::cout << "Invalid scene file.\n";
        return false;
    }

    for (const auto& objectData : sceneData["objects"]) {
        std::string type = objectData.value("type", "");

        if (type == "cube" && c < 100) {
            loadObject(Cu[c], objectData);
            c++;
        }
        else if (type == "sphere" && s < 100) {
            loadObject(Sp[s], objectData);

            if (Sp[s].isLight) {
                int lightIndex = CreatePointLight(Sp[s].position);

                if (lightIndex != -1) {
                    Sp[s].lightIndex = lightIndex;
                }
                else {
                    Sp[s].isLight = false;
                    Sp[s].lightIndex = -1;
                }
            }

            s++;
        }
        else if (type == "cylinder" && y < 100) {
            loadObject(cy[y], objectData);
            y++;
        }
    }

    std::cout << "Scene loaded.\n";
    return true;
}