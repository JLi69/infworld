#version 330 core

out vec4 color;

in float lighting;
in float height;
in vec3 fragpos;

uniform float time;
uniform vec3 lightdir;
uniform vec3 camerapos;

uniform sampler2D sandtexture;
uniform sampler2D grasstexture;
uniform sampler2D stonetexture;
uniform sampler2D snowtexture;

vec4 land()
{
	vec4 sandcolor = texture(sandtexture, fract(fragpos.xz / 16.0));
	vec4 grasscolor = texture(grasstexture, fract(fragpos.xz / 16.0));
	vec4 stonecolor = texture(stonetexture, fract(fragpos.xz / 16.0));
	vec4 snowcolor = texture(snowtexture, fract(fragpos.xz / 16.0));
	sandcolor.a = 0.0;
	grasscolor.a = 0.0;
	stonecolor.a = 0.0;
	snowcolor.a = 0.0;

	return snowcolor * float(height > 0.6) +
		   mix(stonecolor, snowcolor, clamp((height - 0.55) / 0.05, 0.0, 1.0)) * float(height > 0.55 && height <= 0.6) +
		   stonecolor * float(height <= 0.55 && height > 0.3) + 
		   mix(grasscolor, stonecolor, clamp((height - 0.1) / 0.1, 0.0, 1.0)) * float(height > 0.1 && height <= 0.3) +
		   grasscolor * float(height < 0.1 && height > 0.04) +
		   mix(sandcolor, grasscolor, clamp((height - 0.02) / 0.02, 0.0, 1.0)) * float(height > 0.02 && height <= 0.04) +
		   sandcolor * float(height < 0.02);
}

void main()
{
	color = land() * lighting;
	color.a = 1.0;
	//fog
	float d = length(fragpos - camerapos);
	vec4 fogeffect = mix(color, vec4(0.5, 0.8, 1.0, 1.0), min(max(0.0, d - 256.0) / 128.0, 1.0));
	vec4 watereffect = mix(color, vec4(0.1, 0.7, 0.9, 1.0), min(max(0.0, d) / 24.0, 1.0));
	color = fogeffect * float(camerapos.y >= 0.0) + watereffect * float(camerapos.y < 0.0);
}
