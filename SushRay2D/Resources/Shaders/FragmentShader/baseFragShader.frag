#version 430 core

in vec3 fragPos;
in vec2 texCoord;

out vec4 FragColor;

uniform int tileID;

//(float per Pixel) how much 1 pixel is in the 0.0 to 1.0 coordinate system of the static light map on the x and y axis
uniform float fperPix_X;
uniform float fperPix_Y;

//(float per Tile) the distance in the 0.0 to 1.0 coordinate system of the Tileset which is equivalent to 1 tile in the x and y axis
uniform int nrTilesX;
uniform int nrTilesY;

uniform bool hasAlpha;

uniform sampler2D staticLightMap;	//texture0
uniform sampler2D dynamicLightMap;	//texture1
uniform sampler2D tileSetColor;		//texture2


void main()
{
	//get average color of current pixel and the 8 pixels around it on the static light map
	//=====================================================================================
	vec4 staticLightColAverage; 
	int x = 0;
	int y = 0;
	float Xpos = 0;
	float Ypos = 0;
	vec2 tileXY;

	//offset													//scaling of texture coordinates to 1 tile
	//=====================================================		//=====================
	tileXY.x = (1.0 / float(nrTilesX)) * (tileID % nrTilesX) + (texCoord.x / nrTilesX);	//calculate the x coordinate of the current tile on the tileset
	
	if(floor(float(tileID) / nrTilesX) > 0.0)	//check to ensure that there is no division by 0
	{
		//offset															   //scaling of texture coordinates to 1 tile
		//=================================================================	   //=====================
		tileXY.y = (1.0 / float(nrTilesY)) * ceil(float(tileID) / nrTilesX) + (texCoord.y / nrTilesY);	//calculate the y coordinate of the current tile on the tileset
	}
	else
	{
		tileXY.y = 0.0 + (texCoord.y / nrTilesY);	//set y coordinate on tilemap to 0 plus the current texture coordinates of the y axis
	}

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
	//staticLightColAverage += texture(staticLightMap, vec2(Xpos, Ypos));	//add pixel
	//staticLightColAverage += 0.3;
	staticLightColAverage /= 9;
	//=====================================================================================
	//FragColor = staticLightColAverage;
	FragColor = texture(tileSetColor, tileXY) * (staticLightColAverage);
	//Xpos = fragPos.x - ((fperPix_X * 0) - fperPix_X);
	//Ypos = fragPos.y - ((fperPix_Y * 0) - fperPix_Y);
	//FragColor = texture(tileSetColor, tileXY) * texture(staticLightMap, vec2(Xpos, Ypos));
}