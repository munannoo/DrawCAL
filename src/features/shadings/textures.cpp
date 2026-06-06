#include "textures.h"
void loadTexture() {
    Texture2D texture = LoadTexture("../assets/textures/Metal_texture.png");
    if (texture.id == 0) {
        std::cerr << "Failed to load texture: " << "../assets/textures/assets/textures/Metal_texture.png" << std::endl;
    }
    Model model = LoadModelFromMesh(GenMeshPlane(2,2,4,3));
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    Vector3 position = { 10.0f, 0.0f, 0.0f };
    DrawModel(model, position, 1.0f, WHITE);
}