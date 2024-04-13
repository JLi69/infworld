#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 norm;

uniform mat4 persp;
uniform mat4 view;
uniform mat4 transform;

uniform vec3 lightdir;
out float lighting;

out float height;
out vec3 fragpos;
uniform float maxheight;

void main()
{
	height = pos.y / maxheight;
	gl_Position = persp * view * transform * pos;
	fragpos = (transform * pos).xyz;
	lighting = max(-dot(lightdir, norm), 0.0) * 0.5 + 0.5;
}
