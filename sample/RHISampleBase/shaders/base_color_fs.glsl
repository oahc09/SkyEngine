#version 450 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform CameraData {
    mat4 viewProject;
    vec4 position;
} cam;

layout(set = 1, binding = 0) uniform Constant {
    vec4 baseColor;
};

void main()
{
    vec3 V = normalize(cam.position.xyz - inPos);
    vec3 N = normalize(inNormal);
    outColor = vec4(vec3(dot(N, V)), 1);
}