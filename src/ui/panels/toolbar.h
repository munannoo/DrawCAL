#ifndef TOOLBAR_H
#define TOOLBAR_H
#include <string>

typedef struct res {
    int width;
    int height;
    std::string name;   
} res;

// Add enum for readibility in value assigning
enum ResolutionIndex {
    RES_720p,
    RES_900p,
    RES_1080p,
    RES_1440p,
    RES_1600p
};

extern const res cr[5];

void topBar(int &activeRes, bool &dropdownEditMode);

#endif