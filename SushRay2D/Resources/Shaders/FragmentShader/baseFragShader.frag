#version 330 core

in vec3 Pos;

out vec4 FragColor;

void main()
{
	FragColor = vec4(Pos.x, Pos.x, Pos.x, 1.0);
}