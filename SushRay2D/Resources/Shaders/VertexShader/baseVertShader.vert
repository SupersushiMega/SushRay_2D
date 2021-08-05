#version 330 core

layout (location = 0) in vec3 startPos;
out vec3 Pos;

uniform vec2 tileScale;
uniform vec2 translation;

vec3 endPos;

void main()
{
	endPos = (startPos * vec3(tileScale, 1.0)) + vec3(translation, 1.0);
	gl_Position = vec4(endPos, 1.0);
	Pos = (endPos + 1)/2;
}