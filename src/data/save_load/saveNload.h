#ifndef SAVENLOAD_H
#define SAVENLOAD_H

#include <raylib.h>
#include <string>

void saveScene(
    std::string type,
    Color color,
    Vector3 cords,
    Vector3 angles,
    Vector3 transforms
);


#endif