#version 450 core

in vec4 fragColor;
in vec2 uv;

out vec4 frameColor;

uniform sampler2D brush;
uniform float primAlpha;

void main()
{
    vec4 brushColor = texture(brush, uv);
    brushColor.xyz += fragColor.xyz;
    brushColor.xyz = mod(brushColor.xyz, vec3(1.0));
    frameColor = vec4(brushColor.xyz, brushColor.a * primAlpha);
}