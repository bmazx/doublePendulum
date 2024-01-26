#include <vector>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <stdio.h>
#include <stdint.h>

#include "ogls.h"

float vertices[] =
{
	/*      Pos        |    Color */
	 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
	-0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f,
};

uint32_t indices[] = 
{
	0, 1, 3,   // first triangle
	1, 2, 3    // second triangle
};


const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 fragColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
	fragColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core

in vec3 fragColor;

out vec4 outColor;

void main()
{
    outColor = vec4(fragColor, 1.0f);
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argv, char** argc)
{

	GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

	printf("%s\n", "glfw Initialized");

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
		printf("failed to initialize glfw!");
		glfwTerminate();
		return -1;
    }

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("failed to initialize glad!");
		return -1;
	}

	printf("%s\n", "glad Initialized");

	OglsVertexBuffer* vertexBuffer;
	ogls::createVertexBuffer(&vertexBuffer, vertices, sizeof(vertices) / sizeof(float));

	OglsIndexBuffer* indexBuffer;
	ogls::createIndexBuffer(&indexBuffer, indices, sizeof(indices) / sizeof(uint32_t));

	std::vector<OglsVertexArrayAttribute> attributePtrs =
	{
		{ 0, 3, 6 * sizeof(float), Ogls_DataType_Float, (void*)0 },
		{ 1, 3, 6 * sizeof(float), Ogls_DataType_Float, (void*)(3 * sizeof(float)) },
	};

	OglsVertexArrayCreateInfo vertexArrayCreatInfo{};
	vertexArrayCreatInfo.vertexBuffer = vertexBuffer;
	vertexArrayCreatInfo.indexBuffer = indexBuffer;
	vertexArrayCreatInfo.pAttributes = attributePtrs.data();
	vertexArrayCreatInfo.attributeCount = attributePtrs.size();

	OglsVertexArray* vertexArray;
	ogls::createVertexArray(&vertexArray, &vertexArrayCreatInfo);

	uint32_t vertexShader, fragmentShader;

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	uint32_t shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	OglsShaderCreateInfo shaderCreateInfo{};
	shaderCreateInfo.vertexSrc = vertexShaderSource;
	shaderCreateInfo.fragmentSrc = fragmentShaderSource;

	OglsShader* shader;
	ogls::createShaderFromStr(&shader, &shaderCreateInfo);


	glViewport(0, 0, 600, 800);

    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ogls::bindShader(shader);
		ogls::bindVertexArray(vertexArray);
		ogls::renderDrawIndex(ogls::getIndexBufferCount(indexBuffer));

		glfwSwapBuffers(window);
        glfwPollEvents();
    }

	printf("%s\n", "terminating... cleaning up resources");
	ogls::destroyShader(shader);
	ogls::destroyVertexArray(vertexArray);
	ogls::destroyIndexBuffer(indexBuffer);
	ogls::destroyVertexBuffer(vertexBuffer);

    glfwTerminate();
	printf("%s\n", "context terminated");
	
    return 0;
}
