#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inFragTexCoord;

layout (set = 1, binding = 1) uniform sampler2D lightDepthSampler;
layout (set = 2, binding = 0) uniform sampler2D diffuseSampler;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(diffuseSampler, inFragTexCoord);
}