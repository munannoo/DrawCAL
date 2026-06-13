#include <iostream>
#include <fstream>
#include <raylib.h>
#include "json.hpp"

using json = nlohmann::json;

// TODO: object type, object coordinates, transformation angles, scaled transformtions
void saveScene( std::string type, Color color, Vector3 cords, Vector3 angles, Vector3 transforms) {

    try {
        std::ofstream Scene("scene.json");

        if ( Scene.is_open() ) {
            json objectData;

            objectData["type"] = type;
            // objectData["color"] = {color};
            objectData["coordinates"] = {{"x", cords.x}, {"y", cords.y}, {"z", cords.z}};
            objectData["transformations"] = {{"x", angles.x}, {"y", angles.y}, {"z", angles.z}};;
            objectData["scaleTransformations"] = {{"x", transforms.x}, {"y", transforms.y}, {"z", transforms.z}};;

            Scene << objectData.dump(2);
        }
    } catch (const std::runtime_error& e) {
        std::cout << "Error in tryingt to save: " << e.what() << std::endl;
    }
}

// TODO: user's progressions ( quiz )