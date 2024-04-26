#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#include "shader.hpp"
#include "camera.hpp"
#include "infworld.hpp"
#include "gfx.hpp"
#include "app.hpp"

constexpr float SPEED = 16.0f;
constexpr float FLY_SPEED = 10.0f;
constexpr float HEIGHT = 120.0f;
constexpr float SCALE = 2.5f;
constexpr unsigned int RANGE = 7;

int main()
{
	State* state = State::get();
	Camera& cam = state->getCamera();

	//Generate a random seed
	std::random_device rd;
	int seed = rd();
	printf("seed: %d\n", seed);
	infworld::worldseed permutations = infworld::makePermutations(seed, 8);

	//Initialize glfw and glad, if any of this fails, kill the program
	if(!glfwInit())
		die("Failed to init glfw!");
	GLFWwindow* window = glfwCreateWindow(960, 720, "infworld", NULL, NULL);
	if(!window)
		die("Failed to create window!");
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(window, handleWindowResize);
	glfwSetKeyCallback(window, handleKeyInput);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		die("Failed to init glad!");
	initMousePos(window);

	infworld::ChunkTable chunks = infworld::buildWorld(RANGE, permutations, HEIGHT);
	//Quad
	gfx::Vao quad = gfx::createQuadVao();
	//Textures
	unsigned int terraintextures;
	glGenTextures(1, &terraintextures);
	gfx::loadTexture("assets/textures/terraintextures.png", terraintextures);
	unsigned int watermaps[3];
	glGenTextures(3, watermaps);
	gfx::loadTexture("assets/textures/waternormal1.png", watermaps[0]);
	gfx::loadTexture("assets/textures/waternormal2.png", watermaps[1]);
	gfx::loadTexture("assets/textures/waterdudv.png", watermaps[2]);

	ShaderProgram terrainShader("assets/shaders/terrainvert.glsl", "assets/shaders/terrainfrag.glsl");
	ShaderProgram waterShader("assets/shaders/vert.glsl", "assets/shaders/waterfrag.glsl");
	terrainShader.use();
	terrainShader.uniformFloat("maxheight", HEIGHT); 
	terrainShader.uniformFloat("chunksz", CHUNK_SZ);
	terrainShader.uniformInt("prec", PREC);

	glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float dt = 0.0f;
	float time = 0.0f;
	while(!glfwWindowShouldClose(window)) {
		float start = glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get perspective matrix
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		float aspect = float(w) / float(h);
		glm::mat4 persp = glm::perspective(glm::radians(75.0f), aspect, 0.1f, 10000.0f);
		//View matrix
		glm::mat4 view = cam.viewMatrix();
		
		//Draw terrain
		terrainShader.use();
		//Textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terraintextures);
		terrainShader.uniformInt("terraintexture", 0);
		//uniforms
		terrainShader.uniformMat4x4("persp", persp);
		terrainShader.uniformMat4x4("view", view);
		terrainShader.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		terrainShader.uniformVec3("camerapos", cam.position);
		terrainShader.uniformFloat("time", time);
		for(int i = 0; i < chunks.count(); i++) {
			infworld::ChunkPos p = chunks.getPos(i);
			float x = float(p.z) * CHUNK_SZ * 2.0f * float(PREC) / float(PREC + 1);
			float z = float(p.x) * CHUNK_SZ * 2.0f * float(PREC) / float(PREC + 1);
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::scale(transform, glm::vec3(SCALE));
			transform = glm::translate(transform, glm::vec3(x, 0.0f, z));
			terrainShader.uniformMat4x4("transform", transform);
			chunks.bindVao(i);
			chunks.drawVao(i);
		}

		//Draw water
		glDisable(GL_CULL_FACE);
		waterShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, watermaps[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, watermaps[1]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, watermaps[2]);
		waterShader.uniformInt("waternormal1", 0);
		waterShader.uniformInt("waternormal2", 1);
		waterShader.uniformInt("waterdudv", 2);
		waterShader.uniformMat4x4("persp", persp);
		waterShader.uniformMat4x4("view", view);
		waterShader.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		waterShader.uniformVec3("camerapos", cam.position);
		waterShader.uniformFloat("time", time);
		for(int i = 0; i < 9; i++) {
			int ix = i % 3 - 1, iz = i / 3 - 1;
			float x = float(ix) * CHUNK_SZ * 32.0f * 2.0f, z = float(iz) * CHUNK_SZ * 32.0f * 2.0f;
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, glm::vec3(cam.position.x + x, 0.0f, cam.position.z + z));
			transform = glm::scale(transform, glm::vec3(CHUNK_SZ * 32.0f));
			waterShader.uniformMat4x4("transform", transform);
			quad.bind();
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		glEnable(GL_CULL_FACE);

		//Update camera
		cam.position += cam.velocity() * dt * SPEED;
		cam.fly(dt, FLY_SPEED);

		glfwSwapBuffers(window);
		gfx::outputErrors();
		glfwPollEvents();
		time += dt;
		outputFps(dt);
		dt = glfwGetTime() - start;
	}

	//Clean up	
	chunks.clearBuffers();
	gfx::destroyVao(quad);
	glfwTerminate();
}
