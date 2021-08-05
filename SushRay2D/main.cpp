//===============================
// SushRay2D
// Created by SupersushiMega
//===============================

#include <iostream>
#include <math.h>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ShaderLoader/ShaderLoader.h>

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

const uint16_t START_WIDTH = 600;
const uint16_t START_HEIGHT = 600;

const double PI = 2 * acos(0.0f);

uint16_t winWidth = 600;
uint16_t winHeight = 600;

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
	Light(glm::vec2 Location, uint16_t Range, glm::vec3 Color, float Intensitiy, uint16_t RayMult);	//rayMult inicates what multiple of the minimal amount of rays needed for the given range is used
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
	
	void calcLightMap(vector<Light> lights);

	uint16_t width;
	uint16_t height;
	glm::vec3* mapPtr = nullptr;
};

int main()
{
	int32_t status = 0;	//variable for error handling
	TileMap tileMap(100, 100);
	uint8_t x;
	uint8_t y;

	vector<Light> lightList;
	Light tempLight(glm::vec2(300, 300), 100, glm::vec3(1.0f, 1.0f, 1.0f), 0.1f, 2);

	lightList.push_back(tempLight);

	for (x = 0; x < 100; x++)
	{
		for (y = 0; y < 100; y++)
		{
			tileMap.SetTile(x, y, rand() % 2);
		}
	}


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	//set openGL version to 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	//
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	//set openGL profile to core

	GLFWwindow* window = glfwCreateWindow(START_WIDTH, START_HEIGHT, "Sush2D", NULL, NULL);	//create window
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
	staticLightMap.calcLightMap(lightList);
	unsigned int staticLightMapObject;
	float borderCol[] = { 1.0f, 0.0f, 0.0f, 0.0f };	//color outside of map (displayed only when something is displayed outside of map)
	glGenTextures(1, &staticLightMapObject);
	glBindTexture(GL_TEXTURE_2D, staticLightMapObject);
	
	//set textures to clamp to border
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//set border color
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderCol);

	//set filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	//set nearest filter for minifying
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//set linear filter for magnifying

	//get data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, staticLightMap.width, staticLightMap.height, 0, GL_RGB, GL_FLOAT, staticLightMap.mapPtr);
	
	//generate mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
	//==============================================================================================

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);	//set clear color	

	baseShader.use();
	baseShader.setVec2("tileScale", tileMap.tileScale);

	glm::vec3 colList[] = {
		glm::vec3(0.4f, 0.2f, 0.1f),
		glm::vec3(0.8f, 0.8f, 0.8f),
	};
	
	float lastFrame = 0.0f;
	float xTrans = 0.0f;

	uint16_t frameCount = 0;
	glm::vec3 color {0};

	glm::vec2 output;
	while (!glfwWindowShouldClose(window))
	{
		baseShader.setVec2("tileScale", tileMap.tileScale);
		tileMap.calcTileValues(winWidth, winHeight);
		processUserInput(window);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

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
				color = colList[tileMap.mapPtr[(y * tileMap.width) + x]];	//get color
				glUniform2f(glGetUniformLocation(baseShader.ID, "translation"), output.x, output.y);	//set Translation uniform
				glUniform3f(glGetUniformLocation(baseShader.ID, "col"), color.x, color.y, color.z);	//set color
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

Light::Light(glm::vec2 Location, uint16_t Range, glm::vec3 Color, float Intensity, uint16_t RayMult)
{
	location = Location;
	range = Range;
	color = Color;
	intensity = Intensity;
	raycount = (2 * PI * Range) * RayMult;
}

StaticLightMap::StaticLightMap(uint16_t Width, uint16_t Height)
{
	width = Width;
	height = Height;
	mapPtr = new glm::vec3[width * height];
}

void StaticLightMap::calcLightMap(vector<Light> lights)
{
	uint8_t lightID;
	uint32_t rayCntr;
	uint16_t rayDistCntr;
	float angleDelta;
	float angle;
	float lightFalloff;
	glm::vec2 deltaXY;
	glm::vec3 lightCol;
	glm::vec2 rayPos;

	uint16_t rayPosCeil_X;
	uint16_t rayPosCeil_Y;
	
	uint16_t rayPosFloor_X;
	uint16_t rayPosFloor_Y;
	

	for (lightID = 0; lightID < lights.size(); lightID++)
	{
		angle = 0;
		angleDelta = (2 * PI) / lights[lightID].raycount;
		lightFalloff = 1.0f / lights[lightID].range;

		lightCol = lights[lightID].color;// *lights[lightID].intensity;

		for (rayCntr = 0; rayCntr < lights[lightID].raycount; rayCntr++)
		{
			deltaXY.x = sin(angle);
			deltaXY.y = cos(angle);
			rayPos = lights[lightID].location;

			for (rayDistCntr = 0; rayDistCntr < lights[lightID].range; rayDistCntr++)
			{
				//break if outside of map
				//=================
				if (rayPos.y >= height || rayPos.y < 0)
				{
					break;
				}
				else if (rayPos.x >= width || rayPos.x < 0)
				{
					break;
				}
				//=================

				rayPosCeil_X = ceil(rayPos.x);
				rayPosCeil_Y = ceil(rayPos.y);
				rayPosFloor_X = floor(rayPos.x);
				rayPosFloor_Y = floor(rayPos.y);

				mapPtr[(rayPosCeil_X * width) + (rayPosCeil_Y)] = lightCol - (lightFalloff * rayDistCntr);
				mapPtr[(rayPosFloor_X * width) + (rayPosFloor_Y)] = lightCol - (lightFalloff * rayDistCntr);
				//mapPtr[(rayPosFloor_X * width) + (rayPosFloor_Y)] += lightCol;
				//Clamping
				//================================================================
				if (mapPtr[(rayPosCeil_X * width) + (rayPosCeil_Y)].x > 1.0f)
				{
					mapPtr[(rayPosCeil_X * width) + (rayPosCeil_Y)].x = 1.0f;
				}

				if (mapPtr[(rayPosCeil_X * width) + (rayPosCeil_Y)].y > 1.0f)
				{
					mapPtr[(rayPosCeil_X * width) + (rayPosCeil_Y)].y = 1.0f;
				}

				if (mapPtr[(rayPosCeil_X * width) + (rayPosCeil_Y)].z > 1.0f)
				{
					mapPtr[(rayPosCeil_X * width) + (rayPosCeil_Y)].z = 1.0f;
				}


				if (mapPtr[(rayPosFloor_X * width) + (rayPosFloor_Y)].x > 1.0f)
				{										   
					mapPtr[(rayPosFloor_X * width) + (rayPosFloor_Y)].x = 1.0f;
				}

				if (mapPtr[(rayPosFloor_X * width) + (rayPosFloor_Y)].y > 1.0f)
				{
					mapPtr[(rayPosFloor_X * width) + (rayPosFloor_Y)].y = 1.0f;
				}

				if (mapPtr[(rayPosFloor_X * width) + (rayPosFloor_Y)].z > 1.0f)
				{
					mapPtr[(rayPosFloor_X * width) + (rayPosFloor_Y)].z = 1.0f;
				}
				//================================================================
				rayPos += deltaXY;
			}
			angle += angleDelta;
		}
	}
}