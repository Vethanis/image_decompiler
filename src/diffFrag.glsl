#version 450 core

in vec2 fragUv;
out vec4 outColor;

uniform sampler2D A;
uniform sampler2D B;

void main()
{
    vec4 a = texture(A, fragUv);
    vec4 b = texture(B, fragUv);
    vec4 c = abs(a - b);
    c.x = c.x + c.y + c.z;
    outColor = c;
}