#version 330 core

uniform sampler2D tex;

out vec4 color;
in vec2 tc;
in float lighting;

void main()
{
	color = texture(tex, fract(tc));

	//We assume that there are not any transparent texels in the texture
	//other than texels with alpha value of 0 and just discard any texel
	//with alpha value that is not 1.0
	if(color.a < 1.0)
		discard;

	color *= lighting;
	color.a = 1.0;
}
