#ifndef saveNload_h
#define saveNload_h

#include <iostream>
#include <fstream>
#include "objects/object.h"
#include "features/manipulation/Transform.h"
#include "json.hpp"
#include "features/shadings/lighting.h"

void saveScene(
    ObjectInstance Cu[], int c,
    ObjectInstance Sp[], int s,
    ObjectInstance cy[], int y
);

bool loadScene(
    ObjectInstance Cu[], int& c,
    ObjectInstance Sp[], int& s,
    ObjectInstance cy[], int& y
);



// CONTRARY TO STANDARD METHODS, THE SAVE AND LOAD FUNCTIONS ARE ABSOLUTELY ABSENT HERE
// TO FIND OUT THE SAVE AND LOAD FUNCTIONS, LEAD OVER TO OBJECTS.CPP WHERE IT IS DEFINED: save(), load()
    // are those variables not accessable here tho idk 
// C S Y ARE THE COUNTERS FOR CUBES, SPHERES, AND CYLINDERS RESPECTIVELY USED DIRECTLY IN OBJECTS.CPP
// I WONDER IF THAT IS NECSESSARY, COULD WE NOT SIMPLY JUST USE THE SIZE OF THE ARRAYS? 


#endif