#version 400 core

layout (points) in;
layout (triangle_strip, max_vertices = 108) out;

uniform float radius;
uniform int n_segments;
uniform mat4 rotation_matrix;
uniform mat4 scale_matrix;

void create_particle(vec4);

flat in int a_color[];
flat out int i;

void main()
{
	i = a_color[0];
	create_particle(gl_in[0].gl_Position);
}

void create_particle(vec4 origin)
{
	vec4 p = vec4(radius, 0., 0., 0.);
	for (int i = 0; i < n_segments; ++i) {
		gl_Position = scale_matrix * origin;
		EmitVertex();

		gl_Position = scale_matrix * (origin + p);
		EmitVertex();

		p = rotation_matrix * p;
		gl_Position = scale_matrix * (origin + p);
		EmitVertex();
		EndPrimitive();
	}
}
