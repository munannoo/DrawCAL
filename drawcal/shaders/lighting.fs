/*******************************************************************************************
*
*   rPBR [shader] - Physically based rendering fragment shader
*   Adapted for DrawCAL raylib Material/DrawModel pipeline.
*
*   Uses rPBR's Cook-Torrance BRDF functions, Fresnel, GGX distribution,
*   Schlick geometry term, normal mapping, roughness/metalness workflow,
*   parallax helper, HDR tonemapping and gamma correction.
*
*   Original copyright (c) 2017 Victor Fisac
*   License: zlib/libpng
*
**********************************************************************************************/

#version 330

#define MAX_LIGHTS 4
#define MAX_DEPTH_LAYER 20
#define MIN_DEPTH_LAYER 10
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;      // rgb = light color, a = DrawCAL intensity multiplier
    float radius;    // DrawCAL radius/falloff distance for point lights
};

// Input vertex attributes from vertex shader
in vec2 fragTexCoord;
in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBinormal;

// Raylib material maps
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D heightMap;
uniform vec4 colDiffuse;

// rPBR-style lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec3 viewPos;
uniform int renderMode;
uniform vec4 ambientColor;
uniform int useParallaxMapping;
uniform float heightScale;

vec2 texCoord;

const float PI = 3.14159265359;

out vec4 finalColor;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom = a2;
    float denom = (NdotH2*(a2 - 1.0) + 1.0);
    denom = PI*denom*denom;

    return nom/denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = r*r/8.0;

    float nom = NdotV;
    float denom = NdotV*(1.0 - k) + k;

    return nom/denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1*ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0)*pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0)*pow(1.0 - cosTheta, 5.0);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float numLayers = mix(MAX_DEPTH_LAYER, MIN_DEPTH_LAYER, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    float layerDepth = 1.0/numLayers;
    float currentLayerDepth = 0.0;

    vec2 P = viewDir.xy*heightScale;
    vec2 deltaTexCoords = P/numLayers;

    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(heightMap, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(heightMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth/(afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords*weight + currentTexCoords*(1.0 - weight);

    return finalTexCoords;
}

void main()
{
    mat3 TBN = transpose(mat3(fragTangent, fragBinormal, fragNormal));

    vec3 normal = normalize(fragNormal);
    vec3 view = normalize(viewPos - fragPos);

    if (useParallaxMapping == 1 && heightScale > 0.0)
    {
        texCoord = ParallaxMapping(fragTexCoord, view);
    }
    else
    {
        texCoord = fragTexCoord;
    }

    // Texture workflow matches DrawCAL's existing PBR texture loader.
    vec3 color = pow(texture(albedoMap, texCoord).rgb*colDiffuse.rgb, vec3(2.2));
    float metal = clamp(texture(metallicMap, texCoord).r, 0.0, 1.0);
    float rough = clamp(texture(roughnessMap, texCoord).r, 0.04, 1.0);
    vec3 occlusion = texture(aoMap, texCoord).rgb;

    // Normal map fallback is flat blue, so this is safe even when a real normal
    // texture is missing and DrawCAL loaded the fallback texture.
    normal = texture(normalMap, texCoord).rgb;
    normal = normalize(normal*2.0 - 1.0);
    normal = normalize(normal*TBN);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, color, metal);

    vec3 Lo = vec3(0.0);
    vec3 lightDot = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);
            vec3 radiance = lights[i].color.rgb;

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }
            else if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPos);
                float distance = length(lights[i].position - fragPos);

                // Editor-friendly radius falloff: much brighter than strict
                // inverse-square, so one point light can visibly light the object.
                float radius = max(lights[i].radius, 0.01);
                float normalizedDistance = clamp(distance/radius, 0.0, 1.0);
                float rangeFalloff = 1.0 - normalizedDistance*normalizedDistance;
                rangeFalloff *= rangeFalloff;

                float softDistanceFalloff = 1.0/(1.0 + 0.035*distance + 0.006*distance*distance);
                float attenuation = rangeFalloff*softDistanceFalloff;
                radiance *= attenuation;
            }

            vec3 high = normalize(view + light);
            float NDF = DistributionGGX(normal, high, rough);
            float G = GeometrySmith(normal, view, light, rough);
            vec3 F = fresnelSchlick(max(dot(high, view), 0.0), F0);

            vec3 nominator = NDF*G*F;
            float denominator = 4.0*max(dot(normal, view), 0.0)*max(dot(normal, light), 0.0) + 0.001;
            vec3 brdf = nominator/denominator;

            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metal;

            float NdotL = max(dot(normal, light), 0.0);
            float intensity = lights[i].color.a;

            Lo += (kD*color/PI + brdf)*radiance*NdotL*intensity;
            lightDot += radiance*NdotL + brdf*intensity;
        }
    }

    vec3 F = fresnelSchlickRoughness(max(dot(normal, view), 0.0), F0, rough);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metal;

    // DrawCAL does not currently load rPBR's HDR irradiance/prefilter/BRDF maps,
    // so this is the direct-lighting rPBR BRDF with a small ambient fallback.
    vec3 ambient = ambientColor.rgb*kD*color*occlusion;

    vec3 fragmentColor = ambient + Lo;

    // rPBR render debug modes kept for easy defense/testing.
    if (renderMode == 1) fragmentColor = color;          // Albedo
    else if (renderMode == 2) fragmentColor = normal;    // Normals
    else if (renderMode == 3) fragmentColor = vec3(metal);
    else if (renderMode == 4) fragmentColor = vec3(rough);
    else if (renderMode == 5) fragmentColor = occlusion;
    else if (renderMode == 7) fragmentColor = lightDot;
    else if (renderMode == 8) fragmentColor = kS;

    // rPBR HDR tonemapping and gamma correction.
    fragmentColor = fragmentColor/(fragmentColor + vec3(1.0));
    fragmentColor = pow(fragmentColor, vec3(1.0/2.2));

    finalColor = vec4(fragmentColor, colDiffuse.a);
}
