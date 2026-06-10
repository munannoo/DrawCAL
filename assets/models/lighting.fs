#version 330
#define MAX_SHADER_LIGHTS 16
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 viewPos;
uniform vec4 ambientColor;



uniform int lightCount;
uniform vec3 lightPositions[MAX_SHADER_LIGHTS];
uniform vec4 lightColors[MAX_SHADER_LIGHTS];
uniform float lightIntensities[MAX_SHADER_LIGHTS];
uniform float lightRadii[MAX_SHADER_LIGHTS];

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 baseColor = texelColor * colDiffuse;

    vec3 normal = normalize(fragNormal);

    vec3 result = ambientColor.rgb * ambientColor.a * baseColor.rgb;

    for (int i = 0; i < MAX_SHADER_LIGHTS; i++)
{
    if (i >= lightCount) break;

    vec3 toLight = lightPositions[i] - fragPosition;
    float distanceToLight = length(toLight);

    vec3 lightDir = normalize(toLight);

    float diffuseAmount = max(dot(normal, lightDir), 0.0);

    float radius = lightRadii[i];

    float attenuation = 1.0 - clamp(distanceToLight / radius, 0.0, 1.0);
    attenuation = attenuation * attenuation;

    vec3 lightColor = lightColors[i].rgb;
    float intensity = lightIntensities[i];

    vec3 diffuse = baseColor.rgb * lightColor * diffuseAmount * intensity * attenuation;

    result += diffuse;
}

    finalColor = vec4(result, baseColor.a);
}