#version 330

in vec3 vertexPosition;

uniform mat4 mvp;
uniform vec3 cameraPos;

out vec3 fragPos3D;

void main()
{
    vec3 worldPos = vertexPosition + vec3(cameraPos.x, 0.0, cameraPos.z);

    fragPos3D = worldPos;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}