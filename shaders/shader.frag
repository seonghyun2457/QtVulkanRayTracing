#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUv;

layout(location = 0) out vec4 outColor; // Final output color

void main()
{
    outColor = fragColor;
}