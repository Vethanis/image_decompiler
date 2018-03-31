#version 450 core

in vec2 fragUv;
out vec4 outColor;

uniform sampler2D current_frame;
uniform int seed;

float rand( inout uint f) {
    f = (f ^ 61) ^ (f >> 16);
    f *= 9;
    f = f ^ (f >> 4);
    f *= 0x27d4eb2d;
    f = f ^ (f >> 15);
    return fract(float(f) * 2.3283064e-10);
}

void main()
{
    uint s = uint(seed) 
        ^ uint(gl_FragCoord.x * 39163.0) 
        ^ uint(gl_FragCoord.y * 64601.0);
    vec4 color = texture(current_frame, fragUv);
    const float factor = 0.02;
    color.xyz -= vec3(factor);
    color.xyz += vec3(rand(s), rand(s), rand(s)) * factor;
    color.w = 1.0;
    outColor = color;
}