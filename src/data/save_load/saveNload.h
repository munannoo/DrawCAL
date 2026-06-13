#pragma once
#include "objects/object.h"

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