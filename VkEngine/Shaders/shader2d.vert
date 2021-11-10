#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoords;

layout (set = 0, binding = 0) uniform Camera
{
    vec3 pos;
    float aspectRatio;
} camera;

layout (push_constant) uniform PushConstants
{
    vec2 transPos;
    vec2 transScale;
    float transRot;
} pushConstants;

layout(location = 0) out vec2 outFragTexCoord;
layout(location = 1) out vec2 outFragPos;

vec2 rotate(vec2 vec, float rot)
{
    float s = sin(rot);
    float c = cos(rot);

    return vec2(vec.x * c - vec.y * s, vec.x * s + vec.y * c);
}

vec2 get_pos()
{
    vec2 posRotated = rotate(inPosition, pushConstants.transRot);
    vec2 posScaled = posRotated * pushConstants.transScale;
    vec2 translatedPos = pushConstants.transPos + posScaled;
    return translatedPos;
}

vec2 make_camera_relative(vec2 pos)
{
    pos -= vec2(camera.pos.x, camera.pos.y);
    vec2 zoomedPos = pos / (1.0 + camera.pos.z);
    vec2 resPos = vec2(zoomedPos.x, zoomedPos.y * camera.aspectRatio);
    return resPos;
}

void main() 
{
    vec2 pos = get_pos();

    gl_Position = vec4(make_camera_relative(pos), 0.0, 1.0);
    outFragPos = pos;
    outFragTexCoord = inTexCoords;
}