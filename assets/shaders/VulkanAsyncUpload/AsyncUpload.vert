#version 450


layout (location = 0) out vec2 outUv;

vec2 positions[6] = vec2[](
vec2(-0.25, -0.25),
vec2( 0.25, -0.25),
vec2( 0.25,  0.25),
vec2( 0.25,  0.25),
vec2(-0.25,  0.25),
vec2(-0.25, -0.25)
);

vec2 uv[6] = vec2[](
vec2(0.0, 0.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0),

vec2(1.0, 1.0),
vec2(0.0, 1.0),
vec2(0.0, 0.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    outUv = uv[gl_VertexIndex];
}
