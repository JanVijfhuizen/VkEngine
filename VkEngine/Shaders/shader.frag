#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inFragTexCoord;
layout(location = 1) in vec2 inFragPos;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(1, 0, 0, 1);
}