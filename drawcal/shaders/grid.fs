#version 330

in vec3 fragPos3D;
out vec4 finalColor;

uniform vec3 cameraPos;
uniform vec4 gridColor;
uniform vec4 farGridColor;
uniform float scale;

float gridLine(vec3 pos, float spacing)
{
    vec2 coord = pos.xz / spacing;
    vec2 derivative = max(fwidth(coord), vec2(0.000001));
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float lineAlpha = 1.0 - smoothstep(0.0, 1.25, line);
    float density = max(derivative.x, derivative.y);
    float lodFade = 1.0 - smoothstep(0.35, 1.20, density);

    return lineAlpha * lodFade;
}

float axisLine(float value, float spacing)
{
    float d = max(fwidth(value / spacing), 0.000001);
    return 1.0 - smoothstep(0.0, 1.5, abs(value / spacing) / d);
}

void main()
{
    float distanceToCam = length(cameraPos.xz - fragPos3D.xz);
    float distanceFade = 1.0 - clamp(distanceToCam / 200.0, 0.0, 1.0);
    // smaller grid inside the 10*10 square grid
    float fineGrid = gridLine(fragPos3D, scale);
    //Grid thats like every 10 units 
    float majorGrid = gridLine(fragPos3D, scale * 10.0);
    float gridAlpha = max(fineGrid * 0.35, majorGrid);
    vec3 color = mix(farGridColor.rgb, gridColor.rgb, gridAlpha);
    // Axis lines
    float xAxis = axisLine(fragPos3D.z, scale); // red X axis
    float zAxis = axisLine(fragPos3D.x, scale); // green Z axis
    color = mix(color, vec3(1.0, 0.25, 0.25), xAxis);
    color = mix(color, vec3(0.25, 1.0, 0.25), zAxis);
    float axisAlpha = max(xAxis, zAxis);
    float alpha = max(gridAlpha, axisAlpha) * distanceFade * gridColor.a;
    finalColor = vec4(color, alpha);

    if (finalColor.a <= 0.01)
    {
        discard;
    }
}