#include "textures.h"
Texture2D texture = LoadTexture("../assets/textures/Metal_texture.png");
void initTexture(Model &model) {
    if (texture.id == 0) {
        std::cerr << "Failed to load texture: ../assets/textures/Metal_texture.png" << std::endl;
        return;
    }
    if (model.materialCount > 0) {
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    }
}
void UnloadTextures() {
    UnloadTexture(texture);
}