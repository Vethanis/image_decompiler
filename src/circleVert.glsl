#version 450 core

layout(location = 0) in vec4 vertPosition;
layout(location = 1) in vec4 vertColor;

out vec4 fragColor;
out vec2 uv;

void main()
{
    fragColor = vertColor;
    uv = vertPosition.zw;
    gl_Position = vec4(vertPosition.xy, 0.5, 1.0);
}