#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 norm;

uniform mat4 persp;
uniform mat4 view;

uniform vec3 lightdir;
out float lighting;

void main()
{
	gl_Position = persp * view * pos;
	lighting = -dot(lightdir, norm) * 0.4 + 0.6;
}
