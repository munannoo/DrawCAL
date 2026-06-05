#include <raylib.h>
#include "Transform.h"
BoundingBox GetTransformedBounds(BoundingBox baseBox, Vector3 pos, Vector3 scale) {
    BoundingBox transformed;
    // Scale the local boundary reach from center
    transformed.min = { baseBox.min.x * scale.x, baseBox.min.y * scale.y, baseBox.min.z * scale.z };
    transformed.max = { baseBox.max.x * scale.x, baseBox.max.y * scale.y, baseBox.max.z * scale.z };
    
    // Shift boundary coordinates to world space position
    transformed.min = { transformed.min.x + pos.x, transformed.min.y + pos.y, transformed.min.z + pos.z };
    transformed.max = { transformed.max.x + pos.x, transformed.max.y + pos.y, transformed.max.z + pos.z };
    return transformed;
}
