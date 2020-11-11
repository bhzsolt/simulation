#version 400 core

layout(location = 0) in vec2 position;
layout(location = 1) in float color;

flat out int a_color;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	a_color = int(color) - 1;
}
