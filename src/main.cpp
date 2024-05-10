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
#include "arg.hpp"

constexpr float SPEED = 32.0f;
constexpr float FLY_SPEED = 20.0f;
constexpr float HEIGHT = 270.0f;

int main(int argc, char *argv[])
{
	Args argvals = parseArgs(argc, argv);

	State* state = State::get();
	Camera& cam = state->getCamera();

	printf("seed: %d\n", argvals.seed);
	infworld::worldseed permutations = infworld::makePermutations(argvals.seed, 8);

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

	infworld::ChunkTable chunks = 
		infworld::buildWorld(argvals.range, permutations, HEIGHT, CHUNK_SZ);
	//Quad
	gfx::Vao quad = gfx::createQuadVao();
	gfx::Vao cube = gfx::createCubeVao();
	//Cube
	//Textures
	unsigned int terraintextures;
	glGenTextures(1, &terraintextures);
	gfx::loadTexture("assets/textures/terraintextures.png", terraintextures);
	unsigned int watermaps[3];
	glGenTextures(3, watermaps);
	gfx::loadTexture("assets/textures/waternormal1.png", watermaps[0]);
	gfx::loadTexture("assets/textures/waternormal2.png", watermaps[1]);
	gfx::loadTexture("assets/textures/waterdudv.png", watermaps[2]);
	unsigned int skyboxcubemap;
	glGenTextures(1, &skyboxcubemap);
	const std::vector<std::string> faces = {
		"assets/textures/skybox/skybox_east.png",
		"assets/textures/skybox/skybox_west.png",
		"assets/textures/skybox/skybox_up.png",
		"assets/textures/skybox/skybox_down.png",
		"assets/textures/skybox/skybox_north.png",
		"assets/textures/skybox/skybox_south.png"
	};
	gfx::loadCubemap(faces, skyboxcubemap);

	ShaderProgram terrainShader("assets/shaders/terrainvert.glsl", "assets/shaders/terrainfrag.glsl");
	ShaderProgram waterShader("assets/shaders/vert.glsl", "assets/shaders/waterfrag.glsl");
	ShaderProgram skyboxShader("assets/shaders/skyboxvert.glsl", "assets/shaders/skyboxfrag.glsl");
	float viewdist = CHUNK_SZ * SCALE * 2.0f * float(argvals.range) * 0.8f;
	waterShader.use();
	waterShader.uniformFloat("viewdist", viewdist);
	terrainShader.use();
	terrainShader.uniformFloat("viewdist", viewdist);
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
		glm::mat4 persp = glm::perspective(glm::radians(75.0f), aspect, 1.0f, 10000.0f);
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
			glDrawElements(GL_TRIANGLES, CHUNK_VERT_COUNT, GL_UNSIGNED_INT, 0);
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
			float quadscale = CHUNK_SZ * 32.0f * SCALE;
			float x = float(ix) * quadscale * 2.0f, z = float(iz) * quadscale * 2.0f;
			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, glm::vec3(cam.position.x + x, 0.0f, cam.position.z + z));
			transform = glm::scale(transform, glm::vec3(quadscale));
			waterShader.uniformMat4x4("transform", transform);
			quad.bind();
			glDrawElements(GL_TRIANGLES, quad.vertcount, GL_UNSIGNED_INT, 0);
		}
		glEnable(GL_CULL_FACE);

		//Draw skybox
		glCullFace(GL_FRONT);
		glDepthMask(GL_FALSE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxcubemap);
		skyboxShader.use();
		skyboxShader.uniformInt("skybox", 0);
		skyboxShader.uniformMat4x4("persp", persp);
		glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
		skyboxShader.uniformMat4x4("view", skyboxView);
		cube.bind();	
		glDrawElements(GL_TRIANGLES, cube.vertcount, GL_UNSIGNED_INT, 0);
		glDepthMask(GL_TRUE);
		glCullFace(GL_BACK);

		//Update camera
		cam.position += cam.velocity() * dt * SPEED;
		cam.fly(dt, FLY_SPEED);
		chunks.generateNewChunks(cam.position.x, cam.position.z, permutations);

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
