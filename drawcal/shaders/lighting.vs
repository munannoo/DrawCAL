/*******************************************************************************************
*
*   rPBR [shader] - Physically based rendering vertex shader
*   Adapted for DrawCAL raylib Material/DrawModel pipeline.
*
*   Original copyright (c) 2017 Victor Fisac
*   License: zlib/libpng
*
**********************************************************************************************/

#version 330

// Input vertex attributes from raylib
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec3 vertexTangent;

// Input uniform values from raylib
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes to fragment shader
out vec2 fragTexCoord;
out vec3 fragPos;
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBinormal;

void main()
{
    // rPBR-style TBN data for tangent-space normal mapping.
    vec3 vertexBinormal = cross(vertexNormal, vertexTangent);
    mat3 normalMatrix = mat3(matNormal);

    fragPos = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;

    fragNormal = normalize(normalMatrix*vertexNormal);
    fragTangent = normalize(normalMatrix*vertexTangent);
    fragTangent = normalize(fragTangent - dot(fragTangent, fragNormal)*fragNormal);
    fragBinormal = normalize(normalMatrix*vertexBinormal);
    fragBinormal = cross(fragNormal, fragTangent);

    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
