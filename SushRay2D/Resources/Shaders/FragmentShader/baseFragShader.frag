#version 330 core

in vec3 Pos;

out vec4 FragColor;

uniform vec3 col;

void main()
{
	FragColor = vec4(col * Pos.x, 1.0);
}