#version 450
#extension GL_EXT_debug_printf : enable

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;

layout (location = 0) out vec3 out_color;

layout(push_constant) uniform pc {
        mat4 transform;
};


void main()
{
	gl_Position = vec4(in_position, 1.f);
	gl_Position = transform * vec4(in_position, 1.f);

        out_color = in_normal;
}
