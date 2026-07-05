#version 450 // GLSL 4.5

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUv;

void main()
{
    gl_Position = vec4(pos, 1.0);
    fragColor = vec4(col, 1.0);
    fragUv = uv;
}