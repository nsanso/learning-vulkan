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
	debugPrintfEXT("pos: %1.4v4f", vec4(in_position, 1.f));
	gl_Position = vec4(in_position, 1.f);

	debugPrintfEXT("tf0: %1.4v4f", transform[0]);
	debugPrintfEXT("tf1: %1.4v4f", transform[1]);
	debugPrintfEXT("tf2: %1.4v4f", transform[2]);
	debugPrintfEXT("tf3: %1.4v4f", transform[3]);
	gl_Position = transform * vec4(in_position, 1.f);

        out_color = in_color;
}
