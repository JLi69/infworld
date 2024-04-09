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

constexpr float SPEED = 6.0f;
constexpr float FLY_SPEED = 8.0f;
constexpr float HEIGHT = 24.0f;
Camera cam = Camera(glm::vec3(0.0f, 8.0f, 0.0f));
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

mesh::ChunkVaoTable buildWorld(
	unsigned int range,
	const infworld::worldseed &permutations
) {
	mesh::ChunkVaoTable chunkvaos((2 * range + 1) * (2 * range + 1));
	chunkvaos.genBuffers();
	int ind = 0;
	for(int x = -int(range); x <= int(range); x++) {
		for(int z = -int(range); z <= int(range); z++) {
			mesh::ElementArrayBuffer mesh = 
				infworld::createChunkElementArray(permutations, x, z, HEIGHT);
			chunkvaos.addChunk(ind, mesh);
			ind++;
		}
	}
	return chunkvaos;
}

void outputErrors()
{
	GLenum err = glGetError();
	while(err != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error: %d\n", err);
		err = glGetError();
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

	mesh::ChunkVaoTable chunkvaos = buildWorld(15, permutations);

	ShaderProgram program("assets/shaders/vert.glsl", "assets/shaders/frag.glsl");
	program.use();

	program.uniformFloat("maxheight", HEIGHT); 
	program.uniformInt("water1", 0);
	program.uniformInt("water2", 1);

	glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
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
		program.uniformMat4x4("persp", persp);
		program.uniformMat4x4("view", view);
		program.uniformVec3("lightdir", glm::normalize(glm::vec3(-1.0f)));
		program.uniformVec3("camerapos", cam.position);
		program.uniformFloat("time", time);

		for(int i = 0; i < chunkvaos.vaoCount(); i++) {
			chunkvaos.bindVao(i);
			chunkvaos.drawVao(i);
		}

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
	chunkvaos.clearBuffers();
	glfwTerminate();
}
