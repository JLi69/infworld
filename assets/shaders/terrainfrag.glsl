#version 330 core

out vec4 color;

in float lighting;
in float height;
in vec3 fragpos;

uniform float time;
uniform vec3 lightdir;
uniform vec3 camerapos;

uniform sampler2D terraintexture;

vec2 getuv1()
{
	return
		vec2(0.0, 0.0) * float(height < 0.02) +
		vec2(0.25, 0.0) * float(height >= 0.02 && height < 0.1) +
		vec2(0.50, 0.0) * float(height >= 0.1 && height < 0.6) +
		vec2(0.75, 0.0) * float(height >= 0.6);
}

vec2 getuv2()
{
	return
		vec2(0.0, 0.0) * float(height < 0.04) +
		vec2(0.25, 0.0) * float(height >= 0.04 && height < 0.3) +
		vec2(0.50, 0.0) * float(height >= 0.3 && height < 0.7) +
		vec2(0.75, 0.0) * float(height >= 0.7);
}

float mixval(float lower, float upper, float y)
{
	return (1.0 - (y - lower) / (upper - lower)) * float(y >= lower && y < upper);
}

float heightToMix()
{
	return
		mixval(0.02, 0.04, height) +
		mixval(0.1, 0.3, height) +
		mixval(0.6, 0.7, height);
}

vec4 landcolor()
{
	vec2 tc = fract(fragpos.xz / 16.0) * vec2(0.248, 1.0) + vec2(0.001, 0.0);
	vec2 uv1 = getuv1();
	vec2 uv2 = getuv2();

	vec4 color1 = texture(terraintexture, tc + uv1);
	vec4 color2 = texture(terraintexture, tc + uv2);
	color1.a = 0.0;
	color2.a = 0.0;

	return mix(color1, color2, heightToMix());
}

void main()
{
	color = landcolor() * lighting;
	color.a = 1.0;
	//fog
	float d = length(fragpos - camerapos);
	vec4 fogeffect = mix(color, vec4(0.5, 0.8, 1.0, 1.0), min(max(0.0, d - 1024.0) / 128.0, 1.0));
	vec4 watereffect = mix(color, vec4(0.1, 0.7, 0.9, 1.0), min(max(0.0, d) / 24.0, 1.0));
	color = fogeffect * float(camerapos.y >= 0.0) + watereffect * float(camerapos.y < 0.0);
}
