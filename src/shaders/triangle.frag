#version 450

layout (location = 0) in vec3 in_clr;
layout (location = 0) out vec4 out_clr;

void main() {
        out_clr = vec4(in_clr, 1.f);
}
