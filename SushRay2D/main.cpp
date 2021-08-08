//===============================
// SushRay2D
// Created by SupersushiMega
//===============================

#include <iostream>
#include <string>
#include <math.h>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ShaderLoader/ShaderLoader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

const float TILE_VERT[] = {
 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
};

const unsigned int TILE_INDICES[] = {
	0, 1, 3,
	1, 2, 3
};

//prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);	//callback for window resize
void processUserInput(GLFWwindow* window);

using namespace std;	//use std namespace

const uint16_t START_WIDTH = 1000;
const uint16_t START_HEIGHT = 1000;

const double PI = 2 * acos(0.0f);

uint16_t winWidth = 1000;
uint16_t winHeight = 1000;


//TileSets
//=========================================================================================================
//Requires 2 tileSets: Color: how a tile looks and how transparent the tile is based on the alpha channel, Roughness: indicates how reflective a tile is. (Transparency and Roughness use only the outer edges of the Tiles) works without alpha channel(Transparency), Pixeart must be quite large for useable results
/*the tiles are given IDs starting from the top left corner in the following order :  0,  1,  2,  3,
*																					  4,  5,  6,  7,
*																					  8,  9, 10, 11,
*																					 12, 13, 14, 15 */
class TileSet
{
public:
	TileSet(const char *PathColor, const char *PathRoughness, uint8_t NrTilesX, uint8_t NrTilesY, shader& shader);
	~TileSet();
	int32_t Color_width;
	int32_t Color_height;
	int32_t Color_nrChannels;

	int32_t Roughness_width;
	int32_t Roughness_height;
	int32_t Roughness_nrChannels;

	uint8_t nrTilesX;
	uint8_t nrTilesY;
	unsigned char* ColorPtr = nullptr;
	unsigned char* RoughnessPtr = nullptr;
	unsigned int tileSetColor;	//Texture used by openGL
};

class TileMap
{
public:
	TileMap(uint16_t Width, uint16_t Height);
	~TileMap();
	uint8_t GetTile(uint16_t x, uint16_t y);
	void calcTileValues(uint16_t windowWidth, uint16_t windowHeight);	//calculates values used for scaling and positioning of tiles
	void SetTile(uint16_t x, uint16_t y, uint8_t tileID);
	uint16_t width;
	uint16_t height;
	glm::vec2 tileScale;
	glm::vec2 tileDist;
	uint8_t* mapPtr = nullptr;
};

class Light
{
public:
	Light(glm::vec2 Location, uint16_t Range, glm::vec3 Color, float Intensitiy, float RayMult);	//rayMult indicates what multiple of the minimal amount of rays needed for the given range is used
	~Light();
	glm::vec2 location;
	glm::vec3 color;
	uint16_t range;
	float intensity;
	uint32_t raycount;
};

class StaticLightMap
{
public:
	StaticLightMap(uint16_t width, uint16_t height);
	~StaticLightMap();
	
	void bindTileMap(TileMap& tileMap);
	void updateLightMap(vector<Light>& lights);
	void castRay(glm::vec2 rayPos, glm::vec2 deltaXY, glm::vec3& startCol, glm::vec3& lightFalloff, float range, uint8_t bounceCnt);

	uint16_t width;
	uint16_t height;

	glm::vec2 lightTileRatio;
	glm::vec3* mapPtr = nullptr;
	TileMap* tileMapPtr = nullptr;

	uint8_t maxRayBounce = 4;

	unsigned int staticLightMap;	//Texture used by openGL
};

	vector<Light> lightList;
	Light tempLight(glm::vec2(300, 300), 100, glm::vec3(0.5f, 0.5f, 0.5f), 0.1f, 1.0f);
	Light tempLight2(glm::vec2(300, 300), 100, glm::vec3(1.0f, 0.0f, 0.0f), 10.0f, 0.01f);

int main()
{
	int32_t status = 0;	//variable for error handling
	TileMap tileMap(25, 25);
	uint8_t x;
	uint8_t y;


	lightList.push_back(tempLight);
	lightList.push_back(tempLight2);

	for (x = 0; x < 25; x++)
	{
		for (y = 0; y < 25; y++)
		{
			tileMap.SetTile(x, y, rand() % 2);
		}
	}


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	//set openGL version to 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	//
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	//set openGL profile to core

	GLFWwindow* window = glfwCreateWindow(START_WIDTH, START_HEIGHT, "SushRay2D", NULL, NULL);	//create window
	if (window == NULL)	//check for failed window creation
	{
		cout << "GLFW ERROR: Window creation failed" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!status)	//check if GLAD failed to initialize
	{
		cout << "GLAD ERROR: GLAD failed to initialize" << endl;
		return -1;
	}

	glViewport(0, 0, START_WIDTH, START_HEIGHT);	//set size of render window

	//load shaders
	shader baseShader("Resources/Shaders/VertexShader/baseVertShader.vert", "Resources/Shaders/FragmentShader/baseFragShader.frag");
	
	

	//load tileset
	TileSet tileSet("Resources/Textures/TestTiles3x1.png", "Resources/Textures/TestTiles3x1.png_roughness", 3, 1, baseShader);

	//vertex buffer object
	unsigned int VBO;	//Vertex buffer object
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TILE_VERT), TILE_VERT, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);	//define how to read vertex data
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));	//define how to read texture coordinates
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	baseShader.use();

	//Vertex array object
	unsigned int VAO;	//Vertex array object
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TILE_VERT), TILE_VERT, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);	//define how to read vertex data
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));	//define how to read texture coordinates
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//element buffer object
	unsigned int EBO;	//Element buffer object
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TILE_INDICES), TILE_INDICES, GL_STATIC_DRAW);

	//static light map setup
	//==============================================================================================
	StaticLightMap staticLightMap(600, 600);
	staticLightMap.bindTileMap(tileMap);
	staticLightMap.updateLightMap(lightList);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);	//set clear color	

	baseShader.use();
	baseShader.setVec2("tileScale", tileMap.tileScale);
	baseShader.setFloat("fperPix_X", 1.0f / staticLightMap.width);
	baseShader.setFloat("fperPix_Y", 1.0f / staticLightMap.height);

	baseShader.setInt("nrTilesX", tileSet.nrTilesX);
	baseShader.setInt("nrTilesY", tileSet.nrTilesY);

	glm::vec3 colList[] = {
		glm::vec3(0.4f, 0.2f, 0.1f),
		glm::vec3(0.8f, 0.8f, 0.8f),
	};
	
	double lastFrame = 0.0f;
	float xTrans = 0.0f;

	uint16_t frameCount = 0;
	glm::vec3 color {0};

	glm::vec2 output;

	//sampler setup
	baseShader.use();
	baseShader.setInt("staticLightMap", 0);
	baseShader.setInt("dynamicLightMap", 1);
	baseShader.setInt("tileSetColor", 2);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tileSet.tileSetColor);
	while (!glfwWindowShouldClose(window))
	{
		baseShader.setVec2("tileScale", tileMap.tileScale);
		tileMap.calcTileValues(winWidth, winHeight);
		processUserInput(window);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		staticLightMap.updateLightMap(lightList);

		//send data 
		glActiveTexture(GL_TEXTURE0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, staticLightMap.width, staticLightMap.height, 0, GL_RGB, GL_FLOAT, staticLightMap.mapPtr);
		glBindTexture(GL_TEXTURE_2D, staticLightMap.staticLightMap);

		frameCount++;

		if ((glfwGetTime() - lastFrame) >= 1.0f)
		{
			cout << "fps: " << frameCount << endl;
			frameCount = 0;
			lastFrame = glfwGetTime();
		}

		for (x = 0; x < tileMap.width; x++)
		{
			output.x = tileMap.tileScale.x * (x * 2.0f) - tileMap.tileDist.x;	//set x coordinate of translation
			for (y = 0; y < tileMap.height; y++)
			{
				output.y = tileMap.tileScale.y * (y * 2.0f) - tileMap.tileDist.y;	//set y coordinate of translation
				glUniform2f(glGetUniformLocation(baseShader.ID, "translation"), output.x, output.y);	//set Translation uniform
				glUniform1i(glGetUniformLocation(baseShader.ID, "tileID"), tileMap.mapPtr[(y * tileMap.width) + x]);	//set tîleID
				//baseShader.setVec3("col", colList[tileMap.GetTile(x, y)]);
				glDrawElements(GL_TRIANGLES, 8, GL_UNSIGNED_INT, 0);
			}
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	tileMap.~TileMap();
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	winHeight = height;
	winWidth = width;
}

void processUserInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		lightList[0].location.y += 0.5f;
		lightList[1].location.y += 0.5f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		lightList[0].location.y -= 0.5f;
		lightList[1].location.y -= 0.5f;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		lightList[0].location.x += 0.5f;
		lightList[1].location.x += 0.5f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		lightList[0].location.x -= 0.5f;
		lightList[1].location.x -= 0.5f;
	}
}

TileSet::TileSet(const char *PathColor, const char *PathRoughness, uint8_t NrTilesX, uint8_t NrTilesY, shader &shader)
{
	ColorPtr = stbi_load(PathColor, &Color_width, &Color_height, &Color_nrChannels, 0);
	RoughnessPtr = stbi_load(PathRoughness, &Roughness_width, &Roughness_height, &Roughness_nrChannels, 0);
	nrTilesX = NrTilesX;
	nrTilesY = NrTilesY;

	shader.setFloat("fperTileX", 1.0f / NrTilesX);
	shader.setFloat("fperTileY", 1.0f / NrTilesY);

	if (!RoughnessPtr)
	{
		cout << "ERROR: Roughness Tileset failed to load" << endl;
	}

	if (ColorPtr)
	{
		shader.use();
		//Color Tileset setup
		//==============================================================================================
		float borderCol[] = { 1.0f, 0.0f, 0.0f, 0.0f };	//color outside of map (displayed only when something is displayed outside of map)
		glGenTextures(1, &tileSetColor);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, tileSetColor);

		//set textures to clamp to border
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		//set border color
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderCol);

		//set filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	//set nearest filter for minifying
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	//set linear filter for magnifying

		//get data
		glActiveTexture(GL_TEXTURE2);
		if (Color_nrChannels == 4)	//has alpha channel
		{
			shader.setBool("hasAlpha", true);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Color_width, Color_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ColorPtr);
		}
		else if (Color_nrChannels == 3)	//does not have alpha channel (no transparency)
		{
			shader.setBool("hasAlpha", false);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Color_width, Color_height, 0, GL_RGB, GL_UNSIGNED_BYTE, ColorPtr);
		}
		else
		{
			cout << "ERROR: Color Tileset color format not compatible. Must be either RGB or RGBA" << endl;
		}

		//generate mipmap
		glGenerateMipmap(GL_TEXTURE_2D);
		//==============================================================================================
	}
	else
	{
		cout << "ERROR: Color Tileset failed to load" << endl;
	}
}

TileSet::~TileSet()
{
	delete[] ColorPtr;
	delete[] RoughnessPtr;
	ColorPtr = nullptr;
	RoughnessPtr = nullptr;
}

TileMap::TileMap(uint16_t Width, uint16_t Height)
{
	float mapRatio = (float)Width / (float)Height;
	float winRatio = (float)START_WIDTH / (float)START_HEIGHT;

	if (mapRatio < winRatio)	//set scale
	{
		tileScale.x = (1.0f / Height);
		tileScale.y = (1.0f / Height);
	}
	else
	{
		tileScale.x = (1.0f / Width);
		tileScale.y = (1.0f / Width);
	}

	if (winRatio > 1.0f)
	{
		tileScale.x /= winRatio;	//compensate for non square window;
	}
	else if (winRatio < 1.0f)
	{
		tileScale.y *= winRatio;	//compensate for non square window;
	}

	tileDist.x = tileScale.x * (Width - 1);	//set distance between tiles on x axis
	tileDist.y = tileScale.y * (Height - 1);	//set distance between tiles on y axis

	width = Width;
	height = Height;

	mapPtr = new uint8_t[(uint32_t)(Width * Height)]{0};
}

TileMap::~TileMap()
{
	delete[] mapPtr;
	mapPtr = nullptr;
}

uint8_t TileMap::GetTile(uint16_t x, uint16_t y)
{
	//Clamping
	//=================
	if (y > height)
	{
		y = height;
		x = 0;
	}
	else if (x > width)
	{
		x = width;
	}
	//=================

	return mapPtr[(y * width) + x];
}

void TileMap::SetTile(uint16_t x, uint16_t y, uint8_t tileID)
{
	//Clamping
	//=================
	if (y > height)
	{
		y = height;
		x = 0;
	}
	else if (x > width)
	{
		x = width;
	}
	//=================

	mapPtr[(uint32_t)(y * width) + x] = tileID;
}

void TileMap::calcTileValues(uint16_t windowWidth, uint16_t windowHeight)
{
	float mapRatio = (float)width / (float)height;
	float winRatio = (float)windowWidth / (float)windowHeight;

	if (mapRatio < winRatio)	//set scale
	{
		tileScale.x = (1.0f / height);
		tileScale.y = (1.0f / height);
	}
	else
	{
		tileScale.x = (1.0f / width);
		tileScale.y = (1.0f / width);
	}

	if (winRatio > 1.0f)
	{
		tileScale.x /= winRatio;	//compensate for non square window;
	}
	else if (winRatio < 1.0f)
	{
		tileScale.y *= winRatio;	//compensate for non square window;
	}

	tileDist.x = tileScale.x * (width - 1);	//set distance between tiles on x axis
	tileDist.y = tileScale.y * (height - 1);	//set distance between tiles on y axis
}

Light::Light(glm::vec2 Location, uint16_t Range, glm::vec3 Color, float Intensity, float RayMult)
{
	location = Location;
	range = Range;
	color = Color;
	intensity = Intensity;
	raycount = (uint32_t)floor((2 * PI * Range) * RayMult);
}

Light::~Light()
{

}

StaticLightMap::StaticLightMap(uint16_t Width, uint16_t Height)
{
	
	width = Width;
	height = Height;
	mapPtr = new glm::vec3[width * height];
	for (uint32_t i = 0; i < (uint32_t)(width * height); i++)	//set map to 0
	{
		mapPtr[i] = glm::vec3(0.0f);
	}

	//light map setup
	//==============================================================================================
	float borderCol[] = { 1.0f, 0.0f, 0.0f, 0.0f };	//color outside of map (displayed only when something is displayed outside of map)
	glGenTextures(1, &staticLightMap);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, staticLightMap);

	//set textures to clamp to border
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//set border color
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderCol);

	//set filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	//set nearest filter for minifying
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//set linear filter for magnifying

	//get data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, mapPtr);

	//generate mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
	//==============================================================================================
}

StaticLightMap::~StaticLightMap()
{
	delete[] mapPtr;
	mapPtr = nullptr;
	tileMapPtr = nullptr;
}

void StaticLightMap::bindTileMap(TileMap& tileMap)
{
	tileMapPtr = &tileMap;
	lightTileRatio.x = width / tileMapPtr->width;	//the ratio of the x axis in the light map to the x axis of the tile map
	lightTileRatio.y = height / tileMapPtr->height;	//the ratio of the y axis in the light map to the y axis of the tile map
}

void StaticLightMap::updateLightMap(vector<Light>& lights)
{
	uint8_t lightID;
	uint32_t rayCntr;
	float angleDelta;
	float angle;
	glm::vec3 lightFalloff;
	glm::vec3 lightCol;
	glm::vec2 rayPos;
	
	glm::vec2 DeltaXY;

	for (uint32_t i = 0; i < (uint32_t)(width * height); i++)	//set map to 0
	{
		mapPtr[i] = glm::vec3(0.0f);
	}

	for (lightID = 0; lightID < lights.size(); lightID++)
	{
		angle = 0;
		angleDelta = (2 * PI) / lights[lightID].raycount;
		lightCol = lights[lightID].color * lights[lightID].intensity;
		lightFalloff = lightCol / glm::vec3(lights[lightID].range);
		for (rayCntr = 0; rayCntr < lights[lightID].raycount; rayCntr++)
		{
			DeltaXY.x = sin(angle);
			DeltaXY.y = cos(angle);
			castRay(lights[lightID].location, DeltaXY, lightCol, lightFalloff, lights[lightID].range, 0);
			angle += angleDelta;
		}
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, staticLightMap);
}

void StaticLightMap::castRay(glm::vec2 rayPos, glm::vec2 deltaXY, glm::vec3& startCol, glm::vec3& lightFalloff, float range, uint8_t bounceCnt)
{

	bounceCnt++;

	uint16_t rayPosFloor_X;
	uint16_t rayPosFloor_Y;

	uint16_t tileMapX;
	uint16_t tileMapY;

	float TileX;
	float TileY;

	bool insideTile = false;

	bool Yenter = false;
	bool Xenter = false;

	glm::vec3 lightCol = startCol;

	while (range > 0)
	{
		//break if outside of map
		//=================
		if (rayPos.y >= height || rayPos.y < 0)
		{
			return;
		}
		else if (rayPos.x >= width || rayPos.x < 0)
		{
			return;
		}
		//=================

		rayPosFloor_X = (uint16_t)floor(rayPos.x);
		rayPosFloor_Y = (uint16_t)floor(rayPos.y);

		//calculate the xy coordinates of the ray on the tile map
		tileMapX = (uint16_t)floor(rayPos.x / lightTileRatio.x);
		tileMapY = (uint16_t)floor(rayPos.y / lightTileRatio.y);

		if (tileMapPtr->mapPtr[(tileMapY * tileMapPtr->width) + tileMapX])	//check for solid tile
		{
			//calculate the xy coordinates inside the tile
			TileX = abs(((rayPos.x - (lightTileRatio.x / 2)) / lightTileRatio.x) - tileMapX);
			TileY = abs(((rayPos.y - (lightTileRatio.y / 2)) / lightTileRatio.y) - tileMapY);

			if (TileX < TileY)	//check if the collision on the side left or right of the tile
			{
				Xenter = true;
			}
			else if (TileX > TileY)	//check if the collision on the side top or bottom of the tile
			{
				Yenter = true;
			}
			else
			{
				return;
			}

			if ((bounceCnt < maxRayBounce) && !insideTile)
			{
				if (Xenter)	//check if the collision on the side left or right of the tile
				{
					castRay(rayPos - deltaXY, glm::vec2(deltaXY.x, -deltaXY.y), lightCol, lightFalloff, range, bounceCnt);;
				}
				else if (Yenter)	//check if the collision on the side top or bottom of the tile
				{
					castRay(rayPos - deltaXY, glm::vec2(-deltaXY.x, deltaXY.y), lightCol, lightFalloff, range, bounceCnt);
				}
				else
				{
					return;
				}
			}
			else
			{
				//check if the light has gone far enough in the tile to be stopped (number indicates distance from center and ranges from 0.01(middle) to 0.5(outer edge))(WARNING: LOW VALUES LEAD TO UNRELIABILITY)
				float dist = 0.2f;
				if(!(Yenter || Xenter))
				{
					return;
				}
				if (Yenter)
				{
					if (TileX < dist)
					{
						return;
					}
				}
				if (Xenter)
				{
					if (TileY < dist)
					{
						return;
					}
				}
			}
			insideTile = true;
		}
		else if(insideTile)
		{
			return;
		}


		mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)] += lightCol;	//add light to pixel
		rayPos += deltaXY;
		lightCol -= lightFalloff;	//calculate new light color
		range--;
		//Clamping
		//================================================================
		if (mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].x > 1.0f)
		{
			mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].x = 1.0f;
		}
		else if (mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].x < 0.0f)
		{
			mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].x = 0.0f;
		}
		
		
		if (mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].y > 1.0f)
		{
			mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].y = 1.0f;
		}
		else if (mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].y < 0.0f)
		{
			mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].y = 0.0f;
		}
		
		
		if (mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].z > 1.0f)
		{
			mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].z = 1.0f;
		}
		else if (mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].z < 0.0f)
		{
			mapPtr[(rayPosFloor_Y * width) + (rayPosFloor_X)].z = 0.0f;
		}
		//================================================================
	}
	return;
}