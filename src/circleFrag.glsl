#version 450 core

in vec4 fragColor;

out vec4 frameColor;

void main()
{
    frameColor = vec4(fragColor.xyz, 0.5);
}