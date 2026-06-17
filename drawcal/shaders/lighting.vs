#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexTangent;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec4 fragColor;

void main()
{
    vec4 worldPosition = matModel * vec4(vertexPosition, 1.0);

    vec3 N = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
    vec3 T = normalize(vec3(matNormal * vec4(vertexTangent.xyz, 0.0)));

    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T) * vertexTangent.w;

    fragPosition = worldPosition.xyz;
    fragTexCoord = vertexTexCoord;
    fragNormal = N;
    fragTangent = T;
    fragBitangent = B;
    fragColor = vertexColor;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}