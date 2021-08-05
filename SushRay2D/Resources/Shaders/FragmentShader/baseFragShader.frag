#version 330 core

in vec3 fragPos;
in vec2 texCoord;

out vec4 FragColor;

uniform vec3 col;

//(float per Pixel) how much 1 pixel is in the 0.0 to 1.0 coordinate system of the static light map on the x and y axis
uniform float fperPix_X;
uniform float fperPix_Y;

uniform sampler2D staticLightMap;

vec4 staticLightColAverage; 

void main()
{
	//get average color of current pixel and the 8 pixels around it on the static light map
	//=====================================================================================
	int x = 0;
	int y = 0;
	float Xpos = 0;
	float Ypos = 0;
	for(x = 0; x < 3; x++)
	{
		Xpos = fragPos.x - ((fperPix_X * x) - fperPix_X);
		if((Xpos <= 1.0) && (Xpos >= 0.0))	//check if x is outside of static light map
		{
			for(y = 0; y < 3; y++)
			{
				Ypos = fragPos.y - ((fperPix_Y * y) - fperPix_Y);
				if((Ypos <= 1.0) && (Ypos >= 0.0))	//check if y is outside of static light map
				{
					staticLightColAverage += texture(staticLightMap, vec2(Xpos, Ypos));	//add pixel
				}
			}
		}
	}
	staticLightColAverage += 0.3;
	staticLightColAverage /= 9;
	//=====================================================================================
	FragColor = staticLightColAverage * vec4(col, 1.0);
}