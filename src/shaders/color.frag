#version 450

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec4 out_clr;

void main() {
        out_clr = vec4(in_color, 1.f);
}
