#version 400 core

layout(location = 0) out vec4 out_color;

uniform vec3 colors[5];

flat in int i;

void main()
{
	out_color = vec4(colors[i], 1.0);
}
