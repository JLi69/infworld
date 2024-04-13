#version 330 core

out vec4 color;

in vec3 fragpos;

uniform sampler2D waternormal1;
uniform sampler2D waternormal2;
uniform sampler2D waterdudv;

uniform float time;
uniform vec3 lightdir;
uniform vec3 camerapos;

vec2 direction(float angle)
{
	return vec2(cos(angle), sin(angle));
}

void main()
{
	vec2 dudv = (texture(waterdudv, fragpos.xz / 16.0).xy - vec2(0.5, 0.5)) * 2.0;
	vec3 n1 = (texture(waternormal1, fract((fragpos.xz + dudv) / 32.0 + direction(-0.5) * 0.08 * time)).xzy - 0.5) * 2.0,
		 n2 = (texture(waternormal2, fract((fragpos.xz + dudv) / 32.0 + direction(3.14 + 0.5) * 0.05 * time)).xzy - 0.5) * 2.0;
	vec3 normal = normalize(n1 + n2);
	//specular reflection
	vec3 reflected = reflect(lightdir, normal);
	vec3 viewdir = normalize(camerapos - fragpos);
	float specular = pow(max(dot(viewdir, reflected), 0.0), 8.0);
	//diffuse lighting
	float diffuse = max(-dot(lightdir, normal), 0.0) * 0.5 + 0.5;

	color = 
		vec4(0.1, 0.7, 0.9, 0.0) * diffuse + 
		vec4(0.3, 0.3, 0.1, 0.0) * specular;
	color.a = 0.8;

	//fog
	float d = length(fragpos - camerapos);
	vec4 fogeffect = mix(color, vec4(0.5, 0.8, 1.0, 1.0), min(max(0.0, d - 256.0) / 128.0, 1.0));
	vec4 watereffect = mix(color, vec4(0.1, 0.7, 0.9, 1.0), min(max(0.0, d) / 24.0, 1.0));
	color = fogeffect * float(camerapos.y >= 0.0) + watereffect * float(camerapos.y < 0.0);
}
