#version 330

in vec3 vertexPosition;

uniform mat4 matModel;
uniform mat4 pointLightSpaceMatrix;

void main()
{
    gl_Position = pointLightSpaceMatrix * matModel * vec4(vertexPosition, 1.0);
}