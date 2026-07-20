#ifndef saveNload_h
#define saveNload_h

#include <iostream>
#include <fstream>
#include "objects/object.h"
#include "features/manipulation/Transform.h"
#include "json.hpp"
#include "features/shadings/lighting.h"



//void saveScene(
//    ObjectInstance Cu[], int c,
//    ObjectInstance Sp[], int s,
//    ObjectInstance cy[], int y
//);
//
//bool loadScene(
//    ObjectInstance Cu[], int& c,
//    ObjectInstance Sp[], int& s,
//    ObjectInstance cy[], int& y
//);

bool saveScene(const std::string& path = "scene.drawcal");
bool loadScene(const std::string& path = "scene.drawcal");


#endif