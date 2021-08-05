#include "ShaderLoader/ShaderLoader.h"

shader::shader(const char* vertexPath, const char* fragmentPath)
{
	//define strings, stringstreams and ifstreams
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vertShaderFile;
	std::ifstream fragShaderFile;
	std::stringstream vertShaderStream;
	std::stringstream fragShaderStream;

	//check if ifstreams are capable of throwing exceptions
	vertShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	//load data
	//=========================================================================
	try
	{
		//open files
		vertShaderFile.open(vertexPath);
		fragShaderFile.open(fragmentPath);

		//read content in buffer into streams
		vertShaderStream << vertShaderFile.rdbuf();
		fragShaderStream << fragShaderFile.rdbuf();

		//close files
		vertShaderFile.close();
		fragShaderFile.close();

		//convert streams into strings
		vertexCode = vertShaderStream.str();
		fragmentCode = fragShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR: failure loading shader file" << std::endl;
	}
	const char* vertShaderCode = vertexCode.c_str();
	const char* fragShaderCode = fragmentCode.c_str();
	//=========================================================================

	//compilation
	//=========================================================================
	unsigned int vertShader;
	unsigned int fragShader;
	int success;
	char infoLog[512];

	//vertex shader
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertShaderCode, NULL);	//attach code
	glCompileShader(vertShader);	//compile shader

	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);	//get compilation status
	if (!success)	//check if compilation failed
	{
		glGetShaderInfoLog(vertShader, 512, NULL, infoLog);	//get error
		std::cout << "ERROR: compilation of shader failed: \n" << infoLog << std::endl;
	}

	//fragment shader
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragShaderCode, NULL);	//attach code
	glCompileShader(fragShader);	//compile shader

	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);	//get compilation status
	if (!success)	//check if compilation failed
	{
		glGetShaderInfoLog(fragShader, 512, NULL, infoLog);	//get error
		std::cout << "ERROR: compilation of shader failed: \n" << infoLog << std::endl;
	}
	//=========================================================================

	//Linking
	//=========================================================================
	ID = glCreateProgram();
	glAttachShader(ID, vertShader);	//attach vertex shader
	glAttachShader(ID, fragShader);	//attach fragment shader
	glLinkProgram(ID);	//link shaders

	glGetProgramiv(ID, GL_LINK_STATUS, &success);	//get link status
	if (!success)	//check if linking failed
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);	//get error
		std::cout << "ERROR: linking of shaders failed: \n" << infoLog << std::endl;
	}
	
	//delete shaders
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	//=========================================================================
}

void shader::use()
{
	glUseProgram(ID);
}

void shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void shader::setInt(const std::string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void shader::setVec2(const std::string& name, glm::vec2 value) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
}

void shader::setVec3(const std::string& name, glm::vec3 value) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}

void shader::setVec4(const std::string& name, glm::vec4 value) const
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z, value.w);
}

void shader::setMat4(const std::string& name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}