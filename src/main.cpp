#include "glm/fwd.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "ogls.h"

#define PENDULUM_1_MASS   10.0f
#define PENDULUM_2_MASS   10.0f
#define PENDULUM_1_LENGTH 10.0f
#define PENDULUM_2_LENGTH 10.0f
#define PENDULUM_1_ANGLE  90.0f
#define PENDULUM_2_ANGLE  90.0f

#define GRAVITY_CONSTANT  9.81f
#define TIME_STEP         0.0166f

#define COLOR_FG 0.78, 0.82, 1.0
#define COLOR_BG 0.12, 0.11, 0.18
#define COLOR_TRAIL 0.3f, 0.3f, 0.3f

#define PI (22.0f/7.0f) /* 3.1415... */

static const uint32_t s_MaxVertices = 256;
static const uint32_t s_MaxIndices = s_MaxVertices * 8;
static const uint32_t s_MaxTrailVertices = UINT16_MAX;

struct Vertex
{
	OglsVec2 pos;
	OglsVec3 color;
};

struct BatchGroup
{
	OglsVertexBuffer* vertexBuffer;
	OglsIndexBuffer* indexBuffer;
	OglsVertexArray* vertexArray;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 fragColor;

uniform mat4 u_Camera;

void main()
{
	gl_Position = u_Camera * vec4(aPos, 0.0, 1.0);
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

float radians(float deg)
{
	return (deg * PI * 0.005555f);
}

void drawPoly(BatchGroup* batch, OglsVec2 pos, OglsVec3 color, float radius, uint32_t nSides)
{
	batch->vertices.clear();
	batch->indices.clear();

	batch->vertices.reserve(nSides + 1);
	batch->indices.reserve(nSides * 3 + 1);

	batch->vertices.push_back({ pos, color });

	float twoPi = (float)(2 * PI);
	float angle = twoPi / (float)nSides;

	for (uint32_t i = 0; i < nSides; i++)
	{
		float circlex = std::cos(i * angle);
		float circley = std::sin(i * angle);

		float posx = pos.x + (radius * circlex);
		float posy = pos.y + (radius * circley);

		batch->vertices.push_back({{ posx, posy}, color });
		batch->indices.push_back(0);
		batch->indices.push_back(i + 1);
		batch->indices.push_back(i + 2);
	}

	// connect the last vertex off the last triangle to the first vertex on the circle
	batch->indices.back() = batch->indices[1];

	ogls::bindVertexBufferSubData(batch->vertexBuffer, batch->vertices.size() * sizeof(Vertex), 0, (float*)batch->vertices.data());
	ogls::bindIndexBufferSubData(batch->indexBuffer, batch->indices.size() * sizeof(uint32_t), 0, batch->indices.data());

	ogls::bindVertexArray(batch->vertexArray);
	ogls::renderDrawIndex(batch->indices.size());
	ogls::bindVertexArray(0);
}

void drawLine(BatchGroup* batch, OglsVec2 pos1, OglsVec2 pos2, OglsVec3 color)
{
	batch->vertices.clear();
	batch->indices.clear();

	batch->vertices.push_back({ pos1, color });
	batch->vertices.push_back({ pos2, color });
	batch->indices = { 0, 1, 2, 3 };

	ogls::bindVertexBufferSubData(batch->vertexBuffer, batch->vertices.size() * sizeof(Vertex), 0, (float*)batch->vertices.data());
	ogls::bindIndexBufferSubData(batch->indexBuffer, batch->indices.size() * sizeof(uint32_t), 0, batch->indices.data());

	ogls::bindVertexArray(batch->vertexArray);
	ogls::renderDrawIndexMode(GL_LINES, batch->indices.size());
	ogls::bindVertexArray(0);
}

void drawTrail(BatchGroup* batch, OglsVec2 pos, OglsVec3 color)
{
	if (batch->vertices.size() >= s_MaxTrailVertices) { batch->vertices.erase(batch->vertices.begin()); }

	batch->vertices.push_back({pos, color});

	ogls::bindVertexBufferSubData(batch->vertexBuffer, batch->vertices.size() * sizeof(Vertex), 0, (float*)batch->vertices.data());

	ogls::bindVertexArray(batch->vertexArray);
	ogls::renderDrawMode(GL_LINE_STRIP, 0, batch->vertices.size());
	ogls::bindVertexArray(0);
}

float clampAngle(float x)
{
	float angle = std::fmod(x, 2 * PI);
	if (angle < 0) { angle += 2 * PI; }
	return angle;
}

int main(int argv, char** argc)
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	printf("%s\n", "glfw Initialized");

	window = glfwCreateWindow(800, 600, "double pendulum", NULL, NULL);
	if (!window)
	{
		printf("failed to initialize glfw!");
		glfwTerminate();
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("failed to initialize glad!");
		return -1;
	}

	printf("%s\n", "glad Initialized");

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// setup opengl buffers
	std::vector<OglsVertexArrayAttribute> attributePtrs =
	{
		{ 0, 2, sizeof(Vertex), Ogls_DataType_Float, (void*)0 },
		{ 1, 3, sizeof(Vertex), Ogls_DataType_Float, (void*)(2 * sizeof(float)) },
	};


	OglsVertexBuffer* vertexBuffer;
	ogls::createVertexBuffer(&vertexBuffer, nullptr, sizeof(Vertex) * s_MaxVertices, Ogls_BufferMode_Dynamic);

	OglsIndexBuffer* indexBuffer;
	ogls::createIndexBuffer(&indexBuffer, nullptr, sizeof(uint32_t) * s_MaxIndices, Ogls_BufferMode_Dynamic);

	OglsVertexArrayCreateInfo vertexArrayCreatInfo{};
	vertexArrayCreatInfo.vertexBuffer = vertexBuffer;
	vertexArrayCreatInfo.indexBuffer = indexBuffer;
	vertexArrayCreatInfo.pAttributes = attributePtrs.data();
	vertexArrayCreatInfo.attributeCount = attributePtrs.size();

	OglsVertexArray* vertexArray;
	ogls::createVertexArray(&vertexArray, &vertexArrayCreatInfo);


	OglsVertexBuffer* trailVertexBuffer;
	ogls::createVertexBuffer(&trailVertexBuffer, nullptr, sizeof(Vertex) * s_MaxTrailVertices, Ogls_BufferMode_Dynamic);

	OglsVertexArrayCreateInfo trailVertexArrayCreatInfo{};
	trailVertexArrayCreatInfo.vertexBuffer = trailVertexBuffer;
	trailVertexArrayCreatInfo.indexBuffer = nullptr;
	trailVertexArrayCreatInfo.pAttributes = attributePtrs.data();
	trailVertexArrayCreatInfo.attributeCount = attributePtrs.size();

	OglsVertexArray* trailVertexArray;
	ogls::createVertexArray(&trailVertexArray, &trailVertexArrayCreatInfo);


	// setup opengl shader
	OglsShaderCreateInfo shaderCreateInfo{};
	shaderCreateInfo.vertexSrc = vertexShaderSource;
	shaderCreateInfo.fragmentSrc = fragmentShaderSource;

	OglsShader* shader;
	ogls::createShaderFromStr(&shader, &shaderCreateInfo);


	/*
	 * x1 - x pos of first pendulum
	 * y1 - y pos of first pendulum
	 * x2 - x pos of second pendulum
	 * y2 - y pos of second pendulum
	 * a1 - angle of first
	 * a2 - angle of second
	 * l1 - line width of first
	 * l2 - line width of second
	 * m1 - mass of first
	 * m2 - mass of second
	 * av1 - angular velcoity of first
	 * av2 - angular velocity of second
	 * aa1 - angular acceleration of first
	 * aa2 - angular acceleration of second
	 * g - gravitational constant
	 * dt - time step; change in time
	 */
	float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;
	float a1 = 0.0f, a2 = 0.0f, l1 = 0.0f, l2 = 0.0f;
	float m1 = 0.0f, m2 = 0.0f;
	float av1 = 0.0f, av2 = 0.0f, aa1 = 0.0f, aa2 = 0.0f;

	float g = GRAVITY_CONSTANT;
	float dt = TIME_STEP;

	a1 = radians(PENDULUM_1_ANGLE);
	a2 = radians(PENDULUM_2_ANGLE);
	l1 = PENDULUM_1_LENGTH;
	l2 = PENDULUM_2_LENGTH;
	m1 = PENDULUM_1_MASS;
	m2 = PENDULUM_2_MASS;

	glViewport(0, 0, 800, 600);

	BatchGroup batch{};
	batch.vertexBuffer = vertexBuffer;
	batch.indexBuffer = indexBuffer;
	batch.vertexArray = vertexArray;
	
	BatchGroup batchTrail{};
	batchTrail.vertexBuffer = trailVertexBuffer;
	batchTrail.vertexArray = trailVertexArray;

	// imgui settings window
	bool p_open = false, pressOnce = false, gravityOn = true, pause = false, drawTrailPath = false;
	float gChange = g, fov = 60.0f, distance = 50.0f;
	std::string playpause = "play";

	auto timer = std::chrono::high_resolution_clock::now();

	printf("Press the \'c\' key on the keyboard to open the settings\n");

    while (!glfwWindowShouldClose(window))
    {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		int key = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;

		if (key == GLFW_PRESS && !pressOnce)
		{
			if (!p_open)
				p_open = true;
			else
				p_open = false;
			
			pressOnce = true;
		}
		else if (key == GLFW_RELEASE)
			pressOnce = false;

		if (gravityOn) { g = gChange; }
		else { g = 0.0f; }

		// calculate pendulums
		x1 = l1 * std::sin(a1);
		y1 = -l1 * std::cos(a1);
		x2 = x1 + l2 * std::sin(a2);
		y2 = y1 - l2 * std::cos(a2);

		// if pause, skip caululation and render
		if (!pause) 
		{
		float daa1 = (-g * (2 * m1 + m2) * std::sin(a1) - m2 * g * std::sin(a1 - 2 * a2) - 2 * std::sin(a1 - a2) * m2 * (av2 * av2 * l2 + av1 * av1 * l1 * std::cos(a1 - a2))) / (l1 * (2 * m1 + m2 - m2 * std::cos(2 * a1 - 2 * a2)));
		float daa2 = (2 * std::sin(a1 - a2) * (av1 * av1 * l1 * (m1 + m2) + g * (m1 + m2) * std::cos(a1) + av2 * av2 * l2 * m2 * std::cos(a1 - a2))) / (l2 * (2 * m1 + m2 - m2 * std::cos(2 * a1 - 2 * a2)));
		aa1 = daa1 * dt;
		aa2 = daa2 * dt;
		av1 += aa1;
		av2 += aa2;
		a1 += av1 * dt;
		a2 += av2 * dt;
		a1 = clampAngle(a1);
		a2 = clampAngle(a2);
		}

		

		// begin render
		glClearColor(COLOR_BG, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		int width, height;
		glfwGetWindowSize(window, &width, &height);

		glm::mat4 proj = glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, distance + 10.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, distance), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 camera = proj * view * model;

		ogls::bindShader(shader);
		glUniformMatrix4fv(glGetUniformLocation(ogls::getShaderId(shader), "u_Camera"), 1, GL_FALSE, glm::value_ptr(camera));

		// draw trail path
		if (drawTrailPath) { drawTrail(&batchTrail, {x2, y2}, {COLOR_TRAIL}); }

		// draw pendulums
		drawLine(&batch, {0, 0}, {x1, y1}, {COLOR_FG});
		drawLine(&batch, {x1, y1}, {x2, y2}, {COLOR_FG});
		drawPoly(&batch, {x1, y1}, {COLOR_FG}, std::clamp(m1 * 0.1f, 0.1f, 2.0f), 32);
		drawPoly(&batch, {x2, y2}, {COLOR_FG}, std::clamp(m2 * 0.1f, 0.1f, 2.0f), 32);


		if(p_open)
		{
			ImGui::Begin("Settings", &p_open);
			ImGui::Text("Time elapsed: %f", std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - timer).count() * 0.001f * 0.001f * 0.001f);
			ImGui::Text("Pendulum 1:");
			ImGui::Text("  - x1: %f, y1: %f", x1, y1);
			ImGui::Text("  - angle: %f deg (%f rad)", a1 * (180.0f / PI), a1);
			ImGui::Text("  - angular velocity: %f", av1);
			ImGui::Text("  - angular acceleration: %f", aa1);
			ImGui::Text("Pendulum 2:");
			ImGui::Text("  - x2: %f, y2: %f", x2, y2);
			ImGui::Text("  - angle: %f deg (%f rad)", a2 * (180.0f / PI), a2);
			ImGui::Text("  - angular velocity: %f", av2);
			ImGui::Text("  - angular acceleration: %f", aa2);
			ImGui::Spacing();
			float mass[2] = { m1, m2 };
			ImGui::DragFloat2("Pendulum mass", mass, 0.1f, 0.1f, 4096.0f);
			m1 = mass[0]; m2 = mass[1];
			float line[2] = { l1, l2 };
			ImGui::DragFloat2("Pendulum length", line, 0.1f, 0.1f, 4096.0f);
			l1 = line[0]; l2 = line[1];
			float angles[2] = { a1, a2 };
			ImGui::DragFloat2("Pendulum angle", angles, 0.01f, 0.0f, 6.28f);
			a1 = angles[0]; a2 = angles[1];
			ImGui::Spacing();
			ImGui::DragFloat("gravity constant", &gChange, 0.1f);
			
			ImGui::Checkbox("gravity", &gravityOn);
			ImGui::SameLine();
			ImGui::Checkbox("trails", &drawTrailPath);

			if (ImGui::Button(playpause.c_str()))
			{
				pause = !pause;
			}
			if (pause) { playpause = "play"; } else { playpause = "pause"; }

			if (ImGui::Button("randomize length"))
			{
				l1 = 0.1f + (rand() % 50);
				l2 = 0.1f + (rand() % 50);
			}

			ImGui::SameLine();
			
			if (ImGui::Button("randomize mass"))
			{
				m1 = 0.1f + (rand() % 100);
				m2 = 0.1f + (rand() % 100);
			}

			ImGui::SameLine();

			if (ImGui::Button("randomize angles"))
			{
				a1 = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(2 * PI)));
				a2 = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(2 * PI)));
			}

			if (ImGui::Button("randomize"))
			{
				m1 = 0.1f + (rand() % 100);
				m2 = 0.1f + (rand() % 100);
				l1 = 0.1f + (rand() % 50);
				l2 = 0.1f + (rand() % 50);
				a1 = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(2 * PI)));
				a2 = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/(2 * PI)));
			}

			if (ImGui::Button("reset angular velocity"))
			{
				av1 = 0.0f;
				av2 = 0.0f;
			}

			ImGui::SameLine();

			if (ImGui::Button("reset angular acceleration"))
			{
				aa1 = 0.0f;
				aa2 = 0.0f;
			}

			if (ImGui::Button("reset trail path"))
			{
				batchTrail.vertices.clear();
			}

			if (ImGui::Button("reset"))
			{
				m1 = 10.0f; m2 = 10.0f;
				l1 = 10.0f; l2 = 10.0f;
				a1 = radians(90.0f); a2 = radians(90.0f);
				av1 = 0.0f; av2 = 0.0f;
				aa1 = 0.0f; aa2 = 0.0f;
				timer = std::chrono::high_resolution_clock::now();
			}

			ImGui::Spacing();
			ImGui::DragFloat("time step", &dt, 0.001f, 0.0001f, 1.0f, "%.4f");
			
			ImGui::Spacing();
			ImGui::Text("Camera:");
			ImGui::SliderFloat("FOV", &fov, 10.0f, 90.0f);
			ImGui::DragFloat("scale", &distance, 1.0f, 1.0f, 4096.0f);

			ImGui::Spacing();
			ImGui::Text("Info:");
			ImGui::Text("Double Pendulum rendered in OpenGL");
			ImGui::Text("  - Simulates the motion of a double pendulum");
			ImGui::Text("  - Note: degrees/radians start from 0 at the bottom and increase counter-clockwise");

			ImGui::End();
		}


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
        glfwPollEvents();
    }

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	ogls::destroyShader(shader);
	ogls::destroyVertexArray(vertexArray);
	ogls::destroyIndexBuffer(indexBuffer);
	ogls::destroyVertexBuffer(vertexBuffer);
	ogls::destroyVertexBuffer(trailVertexBuffer);
	ogls::destroyVertexArray(trailVertexArray);

    glfwTerminate();
    return 0;
}
