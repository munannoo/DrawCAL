#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec4 lightColor;
uniform vec4 ambientColor;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 baseColor = texelColor * colDiffuse;

    vec3 normal = normalize(fragNormal);

    vec3 lightDirection = normalize(-lightDir);
    float diffuseAmount = max(dot(normal, lightDirection), 0.0);

    vec3 ambient = ambientColor.rgb * ambientColor.a;
    vec3 diffuse = lightColor.rgb * lightColor.a * diffuseAmount;

    vec3 finalRgb = baseColor.rgb * (ambient + diffuse);

    finalColor = vec4(finalRgb, baseColor.a);
}