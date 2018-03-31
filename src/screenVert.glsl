#version 450 core
layout(location = 0) in vec2 uv;
out vec2 fragUv;
void main(){
    gl_Position = vec4(uv, 1.0, 1.0);
    fragUv = uv * 0.5 + 0.5;
    fragUv.y = 1.0 - fragUv.y;
}