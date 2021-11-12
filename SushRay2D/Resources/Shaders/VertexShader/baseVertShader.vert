#version 430 core

layout (location = 0) in vec3 startPos;
layout (location = 1) in vec2 textureCoord;

out vec3 fragPos;
out vec2 texCoord;

uniform vec2 tileScale;
uniform vec2 translation;

vec3 endPos;

void main()
{
	endPos = (startPos * vec3(tileScale, 1.0)) + vec3(translation, 1.0);
	gl_Position = vec4(endPos, 1.0);
	fragPos = (endPos + 1)/2;
	texCoord = textureCoord;
}