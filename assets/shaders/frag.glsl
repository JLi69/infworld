#version 330 core

out vec4 color;

in float lighting;
in float height;

void main()
{
	color = 
		vec4(0.0, 1.0, 0.0, 1.0) * lighting * float(height >= 0.0) +
		vec4(0.0, 0.0, 1.0, 1.0) * float(height < 0.0);
}
