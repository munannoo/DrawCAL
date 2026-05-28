#ifndef TOOLBAR_H
#define TOOLBAR_H
#include <string>

typedef struct res {
    int width;
    int height;
    std::string name;   
} res;

extern const res cr[5];

void topbar(int &activeRes, bool &dropdownEditMode);

#endif