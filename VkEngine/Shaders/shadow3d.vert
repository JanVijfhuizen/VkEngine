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

layout (push_constant) uniform PushConstants
{
    mat4 model;
} pushConstants;

void main() 
{
    gl_Position = camera.projection * camera.view * pushConstants.model * vec4(inPosition, 1);
}