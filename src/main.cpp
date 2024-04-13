#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <stb_image/stb_image.h>

#include "shader.hpp"
#include "camera.hpp"
#include "infworld.hpp"
#include "gfx.hpp"

constexpr float SPEED = 8.0f;
constexpr float FLY_SPEED = 10.0f;
constexpr float HEIGHT = 120.0f;
Camera cam = Camera(glm::vec3(0.0f, 64.0f, 0.0f));
double mousex, mousey;

void die(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

void handleWindowResize(GLFWwindow *window, int w, int h)
{
	glViewport(0, 0, w, h);
}

void handleKeyInput(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	const std::map<int, CameraMovement> keyToMovement = {
		{ GLFW_KEY_W, CameraMovement(FORWARD, NONE, NONE) },
		{ GLFW_KEY_S, CameraMovement(BACKWARD, NONE, NONE) },
		{ GLFW_KEY_A, CameraMovement(NONE, STRAFE_LEFT, NONE) },
		{ GLFW_KEY_D, CameraMovement(NONE, STRAFE_RIGHT, NONE) },
		{ GLFW_KEY_SPACE, CameraMovement(NONE, NONE, FLY_UP) },
		{ GLFW_KEY_LEFT_SHIFT, CameraMovement(NONE, NONE, FLY_DOWN) },
	};

	//Toggle cursor
	if(key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
		glfwSetInputMode(
			window,
			GLFW_CURSOR,
			cursorMode == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED
		);
	}

	if(action == GLFW_PRESS && keyToMovement.count(key))
		cam.updateMovement(keyToMovement.at(key), true);
	else if(action == GLFW_RELEASE && keyToMovement.count(key))
		cam.updateMovement(keyToMovement.at(key), false);	
}

void cursorPosCallback(GLFWwindow *window, double x, double y)
{
	static bool called = false;
	int cursorMode = glfwGetInputMode(window, GLFW_CURSOR);
	double 
		dx = x - mousex,
		dy = y - mousey;
	if(cursorMode == GLFW_CURSOR_DISABLED && called)
		cam.rotateCamera(dx, dy, 0.02f);
	mousex = x;
	mousey = y;
	called = true;
}

mesh::ChunkTable buildWorld(
	unsigned int range,
	const infworld::worldseed &permutations
) {
	double starttime = glfwGetTime();

	mesh::ChunkTable chunks((2 * range + 1) * (2 * range + 1));
	chunks.genBuffers();
	int ind = 0;
	for(int x = -int(range); x <= int(range); x++) {
		for(int z = -int(range); z <= int(range); z++) {
			mesh::ElementArrayBuffer mesh = 
				infworld::createChunkElementArray(permutations, x, z, HEIGHT);
			chunks.addChunk(ind, mesh, x, z);
			ind++;
		}
	}

	double time = glfwGetTime() - starttime;
	printf("Time to generate world: %f\n", time);

	return chunks;
}

void outputErrors()
{
	GLenum err = glGetError();
	int errorcount = 0;
	while(err != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error: %d\n", err);
		err = glGetError();
		errorcount++;
	}

	if(errorcount > 0) {
		fprintf(stderr, "%d error(s)\n", errorcount);
#ifdef DISALLOW_ERRORS 
		die("OpenGL Error, killing program.");
#endif
	}
}

GLenum getFormat(int channels) 
{
	switch(channels) {
	case 1:
		return GL_RED;
	case 3:
		return GL_RGB;
	case 4:
		return GL_RGBA;
	}
	return GL_RGBA;
}

bool loadTexture(const char *path, unsigned int textureid)
{
	int width, height, channels;
	unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
	if(data) {
		GLenum format = getFormat(channels);
		glBindTexture(GL_TEXTURE_2D, textureid);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);	
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
		return true;
	}
	else {
		fprintf(stderr, "Failed to open: %s\n", path);
		stbi_image_free(data);
		return false;
	}
}

int main()
{
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
	glfwGetCursorPos(window, &mousex, &mousey);

	mesh::ChunkTable chunks = buildWorld(7, permutations);

	//Quad
	unsigned int quadbuffers[2];
	unsigned int quadvao;
	glGenVertexArrays(1, &quadvao);
	glBindVertexArray(quadvao);
	glGenBuffers(2, quadbuffers);
	glBindBuffer(GL_ARRAY_BUFFER, quadbuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), QUAD, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadbuffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QUAD_INDICES), QUAD_INDICES, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	//Textures	
	unsigned int textures[4];
	glGenTextures(4, textures);
	loadTexture("assets/textures/sand.png", textures[0]);
	loadTexture("assets/textures/grass.png", textures[1]);
	loadTexture("assets/textures/stone.png", textures[2]);	
	loadTexture("assets/textures/snow.png", textures[3]);	
	unsigned int watermaps[3];
	glGenTextures(3, watermaps);
	loadTexture("assets/textures/waternormal1.png", watermaps[0]);
	loadTexture("assets/textures/waternormal2.png", watermaps[1]);
	loadTexture("assets/textures/waterdudv.png", watermaps[2]);

	ShaderProgram terrainShader("assets/shaders/vert.glsl", "assets/shaders/terrainfrag.glsl");
	ShaderProgram waterShader("assets/shaders/vert.glsl", "assets/shaders/waterfrag.glsl");
	terrainShader.use();
	terrainShader.uniformFloat("maxheight", HEIGHT); 

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
		glm::mat4 persp = glm::perspective(glm::radians(75.0f), aspect, 0.1f, 1000.0f);
		//View matrix
		glm::mat4 view = cam.viewMatrix();
		
		//Draw terrain
		terrainShader.use();
		//Textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textures[3]);
		terrainShader.uniformInt("sandtexture", 0);
		terrainShader.uniformInt("grasstexture", 1);
		terrainShader.uniformInt("stonetexture", 2);
		terrainShader.uniformInt("snowtexture", 3);
		//uniforms
		terrainShader.uniformMat4x4("persp", persp);
		terrainShader.uniformMat4x4("view", view);
		terrainShader.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		terrainShader.uniformVec3("camerapos", cam.position);
		terrainShader.uniformFloat("time", time);
		terrainShader.uniformMat4x4("transform", glm::mat4(1.0f));
		for(int i = 0; i < chunks.count(); i++) {
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
		for(int i = 0; i < chunks.count(); i++) {
			infworld::ChunkPos p = chunks.getPos(i);
			float 
				x = float(p.x) * CHUNK_SZ * 2.0f,
				z = float(p.z) * CHUNK_SZ * 2.0f;

			glm::mat4 transform = glm::mat4(1.0f);
			transform = glm::translate(transform, glm::vec3(x, 0.0f, z));
			transform = glm::scale(transform, glm::vec3(CHUNK_SZ));
			waterShader.uniformMat4x4("transform", transform);
			glBindVertexArray(quadvao);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		glEnable(GL_CULL_FACE);

		//Update camera
		cam.position += cam.velocity() * dt * SPEED;
		cam.fly(dt, FLY_SPEED);

		glfwSwapBuffers(window);
		outputErrors();
		glfwPollEvents();
		time += dt;
		dt = glfwGetTime() - start;
	}

	//Clean up	
	chunks.clearBuffers();
	glfwTerminate();
}
