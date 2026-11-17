#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <shader.hpp>
#include <controls.hpp>
#include <shapes.hpp>
#include <heightmap.hpp>
#include <texture.hpp>
#include <gpuTimer.hpp>

void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
							GLenum severity, GLsizei length,
							const GLchar *message, const void *userParam)
{
	std::cerr << "OpenGL Debug Message:\n";
	std::cerr << "  Source: " << source << "\n";
	std::cerr << "  Type: " << type << "\n";
	std::cerr << "  ID: " << id << "\n";
	std::cerr << "  Severity: " << severity << "\n";
	std::cerr << "  Message: " << message << "\n\n";
}

int main(void)
{
	const int windowWidth = 1920;
	const int windowHeight = 1080;

	GLFWwindow *window;


	if (!glfwInit())
	{
		return -1;
	}
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	window = glfwCreateWindow(windowWidth, windowHeight, "RaymarchingSeasons", monitor, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	int framebufferWidth;
	int framebufferHeight;
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(DebugCallback, nullptr);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glClearColor(0.1f, 0.1f, 0.2f, 0.0f);

	glfwPollEvents();
	glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	// --------------------------------------------------------------------------------------------------
	// Setup
	// --------------------------------------------------------------------------------------------------

	// Framebuffer
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	texture gNormal(framebufferWidth, framebufferHeight, GL_RGB16F, GL_NEAREST, GL_CLAMP_TO_EDGE);
	texture gAlbedo(framebufferWidth, framebufferHeight, GL_RGBA8, GL_NEAREST, GL_CLAMP_TO_EDGE);
	texture gDepth(framebufferWidth, framebufferHeight, GL_DEPTH_COMPONENT32F, GL_NEAREST, GL_CLAMP_TO_EDGE);
	gNormal.attachToFramebuffer(GL_COLOR_ATTACHMENT0);
	gAlbedo.attachToFramebuffer(GL_COLOR_ATTACHMENT1);
	gDepth.attachToFramebuffer(GL_DEPTH_ATTACHMENT);

	GLenum gAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, gAttachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Errore: G-buffer incompleto" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Controlli
	controls controls(window);
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP;

	heightmap perlinHeightmap(2048, 0.125f);

	// --------------------------------------------------------------------------------------------------
	// Terrain
	// --------------------------------------------------------------------------------------------------

	unsigned int vaoTerrain;
	glGenVertexArrays(1, &vaoTerrain);
	glBindVertexArray(vaoTerrain);

	const int terrainPatches = 60;
	const float terrainPatchSize = 4.0f;
	const float terrainForwardOffset = 48.0f;

	std::vector<glm::vec3> positionsTerrain = getOrizzontalSquareV2(terrainPatches, terrainPatchSize);

	unsigned int vboTerrain;
	glGenBuffers(1, &vboTerrain);
	glBindBuffer(GL_ARRAY_BUFFER, vboTerrain);
	glBufferData(GL_ARRAY_BUFFER, positionsTerrain.size() * sizeof(glm::vec3), positionsTerrain.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	unsigned int programTerrain = buildProgram(
							"../src/res/terrain/snapToCamera.vertexshader",
							"../src/res/terrain/quadPatch.tesscontrolshader",
							"../src/res/terrain/displacementMapping.tessevaluationshader",
							"../src/res/terrain/terrain.fragmentshader");
	glUseProgram(programTerrain);

	unsigned int Terrain_u_MVP = glGetUniformLocation(programTerrain, "u_MVP");
	unsigned int Terrain_u_TerrainOffset = glGetUniformLocation(programTerrain, "u_TerrainOffset");

	glUniform1f(glGetUniformLocation(programTerrain, "u_HeightTexelSize"), perlinHeightmap.getTexelSize());
	glUniform1f(glGetUniformLocation(programTerrain, "u_HeightmapWorldSize"), perlinHeightmap.getWorldSize());

	// --------------------------------------------------------------------------------------------------
	// Raymarch Trees
	// --------------------------------------------------------------------------------------------------

	unsigned int vaoFullscreen;
	glGenVertexArrays(1, &vaoFullscreen);

	unsigned int programRaymarchTrees = buildProgram(
							"../src/res/common/fullscreen.vertexshader",
							"../src/res/raymarch/raymarchTrees.fragmentshader");

	unsigned int RaymarchTrees_u_InvVP = glGetUniformLocation(programRaymarchTrees, "u_InvVP");
	unsigned int RaymarchTrees_u_VP = glGetUniformLocation(programRaymarchTrees, "u_VP");
	unsigned int RaymarchTrees_u_CamPos = glGetUniformLocation(programRaymarchTrees, "u_CamPos");
	unsigned int RaymarchTrees_u_Resolution = glGetUniformLocation(programRaymarchTrees, "u_Resolution");
	unsigned int RaymarchTrees_u_RegionCenter = glGetUniformLocation(programRaymarchTrees, "u_RegionCenter");
	unsigned int RaymarchTrees_u_CurrentSeason = glGetUniformLocation(programRaymarchTrees, "u_CurrentSeason");

	glUseProgram(programRaymarchTrees);
	glUniform2f(RaymarchTrees_u_Resolution, static_cast<float>(framebufferWidth), static_cast<float>(framebufferHeight));
	glUniform1f(glGetUniformLocation(programRaymarchTrees, "u_HeightTexelSize"), perlinHeightmap.getTexelSize());
	glUniform1f(glGetUniformLocation(programRaymarchTrees, "u_HeightmapWorldSize"), perlinHeightmap.getWorldSize());

	// --------------------------------------------------------------------------------------------------
	// Raymarch Sky
	// --------------------------------------------------------------------------------------------------

	unsigned int programRaymarchSky = buildProgram(
							"../src/res/common/fullscreen.vertexshader",
							"../src/res/raymarch/raymarchSky.fragmentshader");

	unsigned int RaymarchSky_u_InvVP = glGetUniformLocation(programRaymarchSky, "u_InvVP");
	unsigned int RaymarchSky_u_CamPos = glGetUniformLocation(programRaymarchSky, "u_CamPos");
	unsigned int RaymarchSky_u_Time = glGetUniformLocation(programRaymarchSky, "u_Time");
	glUseProgram(programRaymarchSky);
	glUniform2f(glGetUniformLocation(programRaymarchSky, "u_Resolution"), static_cast<float>(framebufferWidth), static_cast<float>(framebufferHeight));
	glUniform1i(glGetUniformLocation(programRaymarchSky, "u_Depth"), 2);

	// --------------------------------------------------------------------------------------------------
	// Lightning
	// --------------------------------------------------------------------------------------------------

	unsigned int programLighting = buildProgram(
							"../src/res/common/fullscreen.vertexshader",
							"../src/res/lighting/lighting.fragmentshader");

	unsigned int Lighting_u_InvVP = glGetUniformLocation(programLighting, "u_InvVP");
	unsigned int Lighting_u_CamPos = glGetUniformLocation(programLighting, "u_CamPos");
	unsigned int Lighting_u_RegionCenter = glGetUniformLocation(programLighting, "u_RegionCenter");
	glUseProgram(programLighting);
	glUniform2f(glGetUniformLocation(programLighting, "u_Resolution"), static_cast<float>(framebufferWidth), static_cast<float>(framebufferHeight));
	glUniform1f(glGetUniformLocation(programLighting, "u_HeightTexelSize"), perlinHeightmap.getTexelSize());
	glUniform1f(glGetUniformLocation(programLighting, "u_HeightmapWorldSize"), perlinHeightmap.getWorldSize());
	glUniform1i(glGetUniformLocation(programLighting, "u_Normal"), 0);
	glUniform1i(glGetUniformLocation(programLighting, "u_Albedo"), 1);
	glUniform1i(glGetUniformLocation(programLighting, "u_Depth"), 2);

	gpuTimer timerHeightmap;
	gpuTimer timerTerrain;
	gpuTimer timerRaymarchTrees;
	gpuTimer timerRaymarchSky;
	gpuTimer timerLighting;
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glPatchParameteri(GL_PATCH_VERTICES, 4);  
	double cycleStartTime = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		controls.computeMatricesFromInputs();
		glm::vec3 camPos = controls.getPosition();
		glm::vec2 desiredTerrainCenter = glm::vec2(camPos.x, camPos.z) + controls.getHorizontalDirection() * terrainForwardOffset;
		glm::vec2 terrainCenter = glm::floor(desiredTerrainCenter / terrainPatchSize + 0.5f) * terrainPatchSize;
		double currentTime = glfwGetTime();
		double elapsedTime = currentTime - cycleStartTime;

		timerHeightmap.begin();
		perlinHeightmap.update(terrainCenter);
		timerHeightmap.end();

		perlinHeightmap.bindAsSampler(3);

		timerTerrain.begin();
		glUseProgram(programTerrain);
		MVP = controls.getProjectionMatrix() * controls.getViewMatrix() * Model;
		glUniformMatrix4fv(Terrain_u_MVP, 1, GL_FALSE, &MVP[0][0]);
		glUniform2fv(Terrain_u_TerrainOffset, 1, &terrainCenter[0]);
		glBindVertexArray(vaoTerrain);
		glDrawArrays(GL_PATCHES, 0, positionsTerrain.size());
		timerTerrain.end();

		timerRaymarchTrees.begin();
		glUseProgram(programRaymarchTrees);
		glm::mat4 VP = controls.getProjectionMatrix() * controls.getViewMatrix();
		glm::mat4 invVP = glm::inverse(VP);
		glUniformMatrix4fv(RaymarchTrees_u_InvVP, 1, GL_FALSE, &invVP[0][0]);
		glUniformMatrix4fv(RaymarchTrees_u_VP, 1, GL_FALSE, &VP[0][0]);
		glUniform3fv(RaymarchTrees_u_CamPos, 1, &camPos[0]);
		glUniform2fv(RaymarchTrees_u_RegionCenter, 1, &terrainCenter[0]);
		glUniform1f(RaymarchTrees_u_CurrentSeason, static_cast<float>(std::fmod(elapsedTime, 16.0) / 4.0));
		glBindVertexArray(vaoFullscreen);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		timerRaymarchTrees.end();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(vaoFullscreen);

		timerRaymarchSky.begin();
		glUseProgram(programRaymarchSky);
		glUniformMatrix4fv(RaymarchSky_u_InvVP, 1, GL_FALSE, &invVP[0][0]);
		glUniform3fv(RaymarchSky_u_CamPos, 1, &camPos[0]);
		glUniform1f(RaymarchSky_u_Time, static_cast<float>(elapsedTime));
		gDepth.bindAsSampler(2);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		timerRaymarchSky.end();

		timerLighting.begin();
		glUseProgram(programLighting);
		glUniformMatrix4fv(Lighting_u_InvVP, 1, GL_FALSE, &invVP[0][0]);
		glUniform3fv(Lighting_u_CamPos, 1, &camPos[0]);
		glUniform2fv(Lighting_u_RegionCenter, 1, &terrainCenter[0]);
		gNormal.bindAsSampler(0);
		gAlbedo.bindAsSampler(1);
		gDepth.bindAsSampler(2);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		timerLighting.end();

		float totalGpuMs = timerHeightmap.getMs() + timerTerrain.getMs() + timerRaymarchTrees.getMs() + timerRaymarchSky.getMs() + timerLighting.getMs();
		ImGui::Begin("GPU timings", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("Total: %.3f ms", totalGpuMs);
		ImGui::Separator();
		ImGui::Text("Heightmap compute: %.3f ms", timerHeightmap.getMs());
		ImGui::Text("Terrain draw: %.3f ms", timerTerrain.getMs());
		ImGui::Text("Trees raymarch: %.3f ms", timerRaymarchTrees.getMs());
		ImGui::Text("Sky raymarch: %.3f ms", timerRaymarchSky.getMs());
		ImGui::Text("Lighting: %.3f ms", timerLighting.getMs());
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);

		glfwPollEvents();
    }
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}
