#version 330 core

in vec3 fragPos;
in vec2 texCoord;

out vec4 FragColor;

uniform vec3 col;
uniform sampler2D staticLightMap;

void main()
{
	FragColor = texture(staticLightMap, vec2(fragPos.x, fragPos.y)) * vec4(col, 1.0);
}