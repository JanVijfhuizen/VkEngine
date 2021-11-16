#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

layout (set = 0, binding = 0) uniform Camera
{
    mat4 view;
    mat4 projection;
} camera;

layout (set = 1, binding = 0) uniform Light
{
    mat4 spaceMatrix;
} light;

layout (push_constant) uniform PushConstants
{
    mat4 model;
} pushConstants;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outFragTexCoord;

void main() 
{
    gl_Position = camera.projection * camera.view * pushConstants.model * vec4(inPosition, 1);

    outNormal = inNormal;
    outFragTexCoord = inTexCoords;
}