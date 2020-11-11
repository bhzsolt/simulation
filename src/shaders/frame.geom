#version 400 core

layout (points) in;
layout (line_strip, max_vertices = 5) out;

uniform mat4 scale_matrix;

void create_frame(vec4);

void main()
{
	create_frame(gl_in[0].gl_Position);
}

void create_frame(vec4 origin)
{
	gl_Position = scale_matrix * (origin + vec4(-1., -1., 0, 0));
	EmitVertex();
	gl_Position = scale_matrix * (origin + vec4(1., -1., 0, 0));
	EmitVertex();
	gl_Position = scale_matrix * (origin + vec4(1., 1., 0, 0));
	EmitVertex();
	gl_Position = scale_matrix * (origin + vec4(-1., 1., 0, 0));
	EmitVertex();
	gl_Position = scale_matrix * (origin + vec4(-1., -1., 0, 0));
	EmitVertex();
	EndPrimitive();
}
