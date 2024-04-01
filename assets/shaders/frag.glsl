#version 330 core

out vec4 color;

in float lighting;

void main()
{	
	color = vec4(0.0, 1.0, 0.0, 1.0) * lighting;
}
