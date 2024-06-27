#version 330 core

/*
	A vertex shader for trees, takes in vertex position and then translates
	it around to create a wind animation effect
*/

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 norm;

uniform float time;

uniform mat4 persp;
uniform mat4 view;
uniform mat4 transform;

uniform float windstrength;

uniform vec3 lightdir;
out float lighting;

out vec3 fragpos;

out vec2 tc;

void main()
{
	vec4 transformed = transform * pos;
	if(texcoord.x > 0.5) {
		float dist = length(pos.xz);
		float value = sin(pos.x) * 132.0 + cos(pos.z) * 931.0;
		transformed.y += sin(time * dist + value) * 0.03 * dist * windstrength;
		transformed.x += cos(time * dist + value) * 0.03 * dist * windstrength;
		transformed.z += sin(time * dist + value / 2.0) * 0.04 * dist * windstrength;
	}
	gl_Position = persp * view * transformed;
	fragpos = (transform * pos).xyz;
	lighting = max(-dot(lightdir, norm), 0.0) * 0.7 + 0.3;
	tc = texcoord;
}
