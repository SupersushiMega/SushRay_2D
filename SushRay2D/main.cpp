//===============================
// SushRay2D
// Created by SupersushiMega
//===============================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ShaderLoader/ShaderLoader.h>

const float TILE_VERT[] = {
 1.0f,  1.0f, 0.0f, // top right
 1.0f, -1.0f, 0.0f, // bottom right
-1.0f, -1.0f, 0.0f, // bottom left
-1.0f,  1.0f, 0.0f // top lef
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
const uint16_t START_HEIGHT = 800;

class TileMap
{
public:
	TileMap(uint16_t Width, uint16_t Height);
	~TileMap();
	uint8_t GetTile(uint16_t x, uint16_t y);
	void SetTile(uint16_t x, uint16_t y, uint8_t tileID);
	uint16_t width;
	uint16_t height;
	glm::vec2 tileScale;
	uint8_t* MapPtr = nullptr;
};

int main()
{
	int32_t status = 0;	//variable for error handling
	TileMap tileMap(2, 2);

	unsigned int VBO;	//Vertex buffer object
	unsigned int VAO;	//Vertex array object
	unsigned int EBO;	//Element buffer object

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

	status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!status)	//check if GLAD failed to initialize
	{
		cout << "GLAD ERROR: GLAD failed to initialize" << endl;
		return -1;
	}

	//load shaders
	shader baseShader("Resources/Shaders/VertexShader/baseVertShader.vert", "Resources/Shaders/FragmentShader/baseFragShader.frag");

	//vertex buffer object
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TILE_VERT), TILE_VERT, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	baseShader.use();

	//Vertex array object
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TILE_VERT), TILE_VERT, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//element buffer object
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TILE_INDICES), TILE_INDICES, GL_STATIC_DRAW);


	glViewport(0, 0, START_WIDTH, START_HEIGHT);	//set size of render window

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);	//set clear color	

	baseShader.use();
	baseShader.setVec2("tileScale", tileMap.tileScale);

	while (!glfwWindowShouldClose(window))
	{
		processUserInput(window);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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

	if (mapRatio > winRatio)	//set scale
	{
		tileScale.x = (1.0f / Height);
		tileScale.y = (1.0f / Height);
		tileScale.x /= winRatio;	//compensate for non square window;
	}
	else
	{
		tileScale.x = (1.0f / Width);
		tileScale.y = (1.0f / Width);
		tileScale.x /= winRatio;	//compensate for non square window;
	}


	width = Width;
	height = Height;

	MapPtr = new uint8_t[(uint32_t)(Width * height)]{0};
}

TileMap::~TileMap()
{
	delete[] MapPtr;
	MapPtr = nullptr;
}

uint8_t TileMap::GetTile(uint16_t x, uint16_t y)
{
	//Clamping
	//=================
	if (x > width)
	{
		x = width;
	}

	if (y > height)
	{
		y = height;
	}
	//=================

	return MapPtr[(y * width) + x];
}

void TileMap::SetTile(uint16_t x, uint16_t y, uint8_t tileID)
{
	//Clamping
	//=================
	if (x > width)
	{
		x = width;
	}

	if (y > height)
	{
		y = height;
		x = 0;
	}
	//=================

	MapPtr[(uint32_t)(y * width) + x] = tileID;
}