#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 norm;

uniform mat4 persp;
uniform mat4 view;

uniform vec3 lightdir;
out float lighting;

out float height;
out vec3 fragpos;
uniform float maxheight;

void main()
{
	height = pos.y / maxheight;
	float y = max(0.0, pos.y);
	gl_Position = persp * view * vec4(pos.x, y, pos.z, 1.0);
	fragpos = vec3(pos.x, y, pos.z);
	lighting = -dot(lightdir, norm) * 0.3 + 0.7;
}
