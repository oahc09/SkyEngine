#version 450

layout(location = 0) in vec2 offset;
layout(location = 1) in vec2 scale;

layout(location = 0) out vec4 outColor;

vec2 positions[3] = vec2[](
vec2(-0.5,  -0.5),
vec2( 0.5,  -0.5),
vec2( 0.0,   0.5)
);

vec4 color[3] = vec4[](
vec4(1.0, 0.0, 0.0, 1.0),
vec4(0.0, 1.0, 0.0, 1.0),
vec4(0.0, 0.0, 1.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex] * scale + offset, 0.0, 1.0);
    outColor = color[gl_VertexIndex];
}