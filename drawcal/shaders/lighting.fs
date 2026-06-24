#version 330

#define MAX_SHADER_LIGHTS 64
#define MAX_RAYTRACE_OBJECTS 64
#define PI 3.14159265359

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D heightMap;

uniform vec4 colDiffuse;

uniform vec3 viewPos;
uniform vec4 ambientColor;

uniform float heightScale;
uniform int useParallaxMapping;

uniform int lightCount;
uniform vec3 lightPositions[MAX_SHADER_LIGHTS];
uniform vec4 lightColors[MAX_SHADER_LIGHTS];
uniform float lightIntensities[MAX_SHADER_LIGHTS];
uniform float lightRadii[MAX_SHADER_LIGHTS];

// Toggle for the new shader ray-traced shadows.
uniform int rayTracingEnabled;

// Scene primitives uploaded by object.cpp for ray-traced shadows.
uniform int rtCubeCount;
uniform vec3 rtCubePositions[MAX_RAYTRACE_OBJECTS];
uniform vec3 rtCubeRotations[MAX_RAYTRACE_OBJECTS];
uniform vec3 rtCubeScales[MAX_RAYTRACE_OBJECTS];

uniform int rtSphereCount;
uniform vec3 rtSpherePositions[MAX_RAYTRACE_OBJECTS];
uniform vec3 rtSphereScales[MAX_RAYTRACE_OBJECTS];

uniform int rtCylinderCount;
uniform vec3 rtCylinderPositions[MAX_RAYTRACE_OBJECTS];
uniform vec3 rtCylinderRotations[MAX_RAYTRACE_OBJECTS];
uniform vec3 rtCylinderScales[MAX_RAYTRACE_OBJECTS];

// Legacy 360 point-light shadow uniforms. These are used only when ray tracing is OFF.
uniform int usePointLightShadow;
uniform int shadowPointLightIndex;
uniform vec3 shadowPointLightPosition;

uniform float shadowCameraMaxDistance;
uniform float shadowCameraFadeRange;

uniform sampler2D pointShadowMap0;
uniform sampler2D pointShadowMap1;
uniform sampler2D pointShadowMap2;
uniform sampler2D pointShadowMap3;
uniform sampler2D pointShadowMap4;
uniform sampler2D pointShadowMap5;

uniform mat4 pointLightSpaceMatrices[6];

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    if (useParallaxMapping == 0 || heightScale <= 0.0001)
    {
        return texCoords;
    }

    if (viewDir.z <= 0.25)
    {
        return texCoords;
    }

    float height = texture(heightMap, texCoords).r;
    float centeredHeight = height - 0.5;

    vec2 offset = viewDir.xy / max(viewDir.z, 0.25) * centeredHeight * heightScale;

    return texCoords - offset;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return a2 / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    return NdotV / max(NdotV * (1.0 - k) + k, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 RotateX(vec3 v, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return vec3(v.x, c * v.y - s * v.z, s * v.y + c * v.z);
}

vec3 RotateY(vec3 v, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return vec3(c * v.x + s * v.z, v.y, -s * v.x + c * v.z);
}

vec3 RotateZ(vec3 v, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return vec3(c * v.x - s * v.y, s * v.x + c * v.y, v.z);
}

vec3 InverseRotateXYZ(vec3 v, vec3 rotationDegrees)
{
    vec3 r = -radians(rotationDegrees);

    // Inverse of the editor's X/Y/Z object rotation, good enough for shadow rays.
    v = RotateZ(v, r.z);
    v = RotateY(v, r.y);
    v = RotateX(v, r.x);

    return v;
}

float SafeInverse(float v)
{
    if (abs(v) < 0.00001)
    {
        return v < 0.0 ? -100000.0 : 100000.0;
    }

    return 1.0 / v;
}

bool RaySphereHit(vec3 rayOrigin, vec3 rayDir, vec3 center, vec3 scale, float maxDistance)
{
    float radius = max(max(scale.x, scale.y), scale.z);
    radius = max(radius, 0.001);

    vec3 oc = rayOrigin - center;
    float b = dot(oc, rayDir);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c;

    if (h < 0.0) return false;

    h = sqrt(h);

    float t = -b - h;
    if (t > 0.03 && t < maxDistance - 0.03) return true;

    t = -b + h;
    return t > 0.03 && t < maxDistance - 0.03;
}

bool RayBoxHit(vec3 rayOrigin, vec3 rayDir, vec3 position, vec3 rotation, vec3 scale, float maxDistance)
{
    vec3 ro = InverseRotateXYZ(rayOrigin - position, rotation);
    vec3 rd = normalize(InverseRotateXYZ(rayDir, rotation));

    vec3 halfSize = max(abs(scale), vec3(0.001));

    vec3 invD = vec3(SafeInverse(rd.x), SafeInverse(rd.y), SafeInverse(rd.z));

    vec3 t0 = (-halfSize - ro) * invD;
    vec3 t1 = ( halfSize - ro) * invD;

    vec3 tsmaller = min(t0, t1);
    vec3 tbigger = max(t0, t1);

    float tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    float tmax = min(min(tbigger.x, tbigger.y), tbigger.z);

    if (tmax < 0.03) return false;
    if (tmin > tmax) return false;

    float t = tmin > 0.03 ? tmin : tmax;
    return t > 0.03 && t < maxDistance - 0.03;
}

bool RayCylinderHit(vec3 rayOrigin, vec3 rayDir, vec3 position, vec3 rotation, vec3 scale, float maxDistance)
{
    vec3 ro = InverseRotateXYZ(rayOrigin - position, rotation);
    vec3 rd = normalize(InverseRotateXYZ(rayDir, rotation));

    float radius = max(max(abs(scale.x), abs(scale.z)), 0.001);
    float halfHeight = max(abs(scale.y), 0.001);

    float a = rd.x * rd.x + rd.z * rd.z;
    float b = 2.0 * (ro.x * rd.x + ro.z * rd.z);
    float c = ro.x * ro.x + ro.z * ro.z - radius * radius;

    if (abs(a) > 0.00001)
    {
        float disc = b * b - 4.0 * a * c;

        if (disc >= 0.0)
        {
            float root = sqrt(disc);
            float t0 = (-b - root) / (2.0 * a);
            float t1 = (-b + root) / (2.0 * a);

            float y0 = ro.y + t0 * rd.y;
            if (t0 > 0.03 && t0 < maxDistance - 0.03 && y0 >= -halfHeight && y0 <= halfHeight)
            {
                return true;
            }

            float y1 = ro.y + t1 * rd.y;
            if (t1 > 0.03 && t1 < maxDistance - 0.03 && y1 >= -halfHeight && y1 <= halfHeight)
            {
                return true;
            }
        }
    }

    // Top and bottom caps.
    if (abs(rd.y) > 0.00001)
    {
        float tTop = (halfHeight - ro.y) / rd.y;
        vec3 pTop = ro + rd * tTop;
        if (tTop > 0.03 && tTop < maxDistance - 0.03 && dot(pTop.xz, pTop.xz) <= radius * radius)
        {
            return true;
        }

        float tBottom = (-halfHeight - ro.y) / rd.y;
        vec3 pBottom = ro + rd * tBottom;
        if (tBottom > 0.03 && tBottom < maxDistance - 0.03 && dot(pBottom.xz, pBottom.xz) <= radius * radius)
        {
            return true;
        }
    }

    return false;
}

float CameraShadowFade(vec3 fragPos)
{
    float cameraDist = length(viewPos - fragPos);

    return 1.0 - smoothstep(
        shadowCameraMaxDistance,
        shadowCameraMaxDistance + shadowCameraFadeRange,
        cameraDist
    );
}

float CalculateRayTracedShadow(vec3 fragPos, vec3 geometricNormal, vec3 lightPosition)
{
    float cameraFade = CameraShadowFade(fragPos);

    if (cameraFade <= 0.001)
    {
        return 0.0;
    }

    vec3 origin = fragPos + geometricNormal * 0.08;
    vec3 toLight = lightPosition - origin;
    float maxDistance = length(toLight);

    if (maxDistance <= 0.05)
    {
        return 0.0;
    }

    vec3 rayDir = toLight / maxDistance;

    for (int i = 0; i < MAX_RAYTRACE_OBJECTS; i++)
    {
        if (i >= rtCubeCount) break;
        if (RayBoxHit(origin, rayDir, rtCubePositions[i], rtCubeRotations[i], rtCubeScales[i], maxDistance))
        {
            return cameraFade;
        }
    }

    for (int i = 0; i < MAX_RAYTRACE_OBJECTS; i++)
    {
        if (i >= rtSphereCount) break;
        if (RaySphereHit(origin, rayDir, rtSpherePositions[i], rtSphereScales[i], maxDistance))
        {
            return cameraFade;
        }
    }

    for (int i = 0; i < MAX_RAYTRACE_OBJECTS; i++)
    {
        if (i >= rtCylinderCount) break;
        if (RayCylinderHit(origin, rayDir, rtCylinderPositions[i], rtCylinderRotations[i], rtCylinderScales[i], maxDistance))
        {
            return cameraFade;
        }
    }

    return 0.0;
}

int GetPointShadowFace(vec3 direction)
{
    vec3 absDir = abs(direction);

    if (absDir.x >= absDir.y && absDir.x >= absDir.z)
    {
        return direction.x > 0.0 ? 0 : 1;
    }
    else if (absDir.y >= absDir.x && absDir.y >= absDir.z)
    {
        return direction.y > 0.0 ? 2 : 3;
    }
    else
    {
        return direction.z > 0.0 ? 4 : 5;
    }
}

float SamplePointShadowMap(int face, vec2 uv)
{
    if (face == 0) return texture(pointShadowMap0, uv).r;
    if (face == 1) return texture(pointShadowMap1, uv).r;
    if (face == 2) return texture(pointShadowMap2, uv).r;
    if (face == 3) return texture(pointShadowMap3, uv).r;
    if (face == 4) return texture(pointShadowMap4, uv).r;

    return texture(pointShadowMap5, uv).r;
}

float CalculatePointShadow(vec3 fragPos, vec3 N, vec3 L)
{
    float cameraShadowFade = CameraShadowFade(fragPos);

    if (cameraShadowFade <= 0.001)
    {
        return 0.0;
    }

    vec3 lightToFrag = fragPos - shadowPointLightPosition;

    int face = GetPointShadowFace(lightToFrag);

    vec4 lightSpacePos = pointLightSpaceMatrices[face] * vec4(fragPos, 1.0);

    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 0.0;
    }

    float currentDepth = projCoords.z;

    float bias = max(0.008 * (1.0 - dot(N, L)), 0.002);

    vec2 texelSize = 1.0 / vec2(textureSize(pointShadowMap0, 0));

    float shadow = 0.0;
    float samples = 0.0;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 sampleUV = projCoords.xy + vec2(x, y) * texelSize;

            if (sampleUV.x < 0.0 || sampleUV.x > 1.0 ||
                sampleUV.y < 0.0 || sampleUV.y > 1.0)
            {
                continue;
            }

            float closestDepth = SamplePointShadowMap(face, sampleUV);

            if (currentDepth - bias > closestDepth)
            {
                shadow += 1.0;
            }

            samples += 1.0;
        }
    }

    if (samples <= 0.0)
    {
        return 0.0;
    }

    shadow /= samples;

    return shadow * cameraShadowFade;
}

void main()
{
    vec3 N0 = normalize(fragNormal);
    vec3 T = normalize(fragTangent);
    vec3 B = normalize(fragBitangent);

    mat3 TBN = mat3(T, B, N0);

    vec3 V = normalize(viewPos - fragPosition);
    vec3 viewDirTangent = normalize(transpose(TBN) * V);

    vec2 uv = ParallaxMapping(fragTexCoord, viewDirTangent);

    vec3 albedo = texture(albedoMap, uv).rgb * colDiffuse.rgb;
    vec3 normalTex = texture(normalMap, uv).rgb;

    float metallic = texture(metallicMap, uv).r;
    float roughness = texture(roughnessMap, uv).r;
    float ao = texture(aoMap, uv).r;

    normalTex = normalTex * 2.0 - 1.0;

    vec3 N = normalize(TBN * normalTex);

    roughness = clamp(roughness, 0.04, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < MAX_SHADER_LIGHTS; i++)
    {
        if (i >= lightCount) break;

        vec3 L = normalize(lightPositions[i] - fragPosition);
        vec3 H = normalize(V + L);

        float distanceToLight = length(lightPositions[i] - fragPosition);

        float rangeFade = 1.0 - clamp(distanceToLight / lightRadii[i], 0.0, 1.0);
        rangeFade *= rangeFade;

        float inverseSquare = 1.0 / max(distanceToLight * distanceToLight, 1.0);

        float attenuation = rangeFade * inverseSquare;

        vec3 radiance = lightColors[i].rgb * lightIntensities[i] * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;

        float denominator =
            4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;

        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        float shadow = 0.0;

        if (rayTracingEnabled == 1)
        {
            shadow = CalculateRayTracedShadow(fragPosition, N0, lightPositions[i]);
        }
        else if (usePointLightShadow == 1 && i == shadowPointLightIndex)
        {
            shadow = CalculatePointShadow(fragPosition, N, L);
        }

        float visibility = 1.0 - shadow;

        Lo += visibility *
              (kD * albedo / PI + specular) *
              radiance *
              NdotL;
    }

    vec3 ambient = ambientColor.rgb * ambientColor.a * albedo * ao;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    finalColor = vec4(color, colDiffuse.a);
}
