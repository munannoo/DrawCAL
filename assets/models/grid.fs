#version 330
in vec3 fragPos3D;
out vec4 finalColor;
uniform vec3 cameraPos;
uniform vec4 gridColor;
uniform vec4 farGridColor;
uniform float scale;
vec4 grid(vec3 pos, float scale, bool drawAxis)
{
    vec2 coord = pos.xz / scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float gridLine = 1.0 - smoothstep(0.0, 1.5, line);
    float distanceToCam = length(cameraPos.xz - pos.xz);
    float fadeFactor = 1.0 - clamp(distanceToCam / 200.0, 0.0, 1.0);
    vec4 c = mix(farGridColor, gridColor, gridLine);
    c.a *= gridLine * fadeFactor;

    if (drawAxis){
        float xAxis = 1.0 - smoothstep(0.0, 1.5, abs(pos.z) / derivative.y);
        float zAxis = 1.0 - smoothstep(0.0, 1.5, abs(pos.x) / derivative.x);
        // Fade with the grid
        xAxis *= fadeFactor;
        zAxis *= fadeFactor;
        // Red axis
        c.rgb = mix(c.rgb, vec3(1.0, 0.25, 0.25), xAxis);
        // Green axis
        c.rgb = mix(c.rgb, vec3(0.25, 1.0, 0.25), zAxis);
    }
    return c;
}
void main()
{
    finalColor = grid(fragPos3D, scale, true);
    if (finalColor.a <= 0.01)
    {
        discard;
    }
}