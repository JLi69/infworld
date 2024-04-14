#pragma once
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>

namespace mesh {
	template<typename T>
	struct Mesh {
		std::vector<T> vertices;
	};

	//Mesh of floats
	typedef Mesh<float> Meshf;

	struct ElementArrayBuffer {
		Meshf mesh;
		std::vector<unsigned int> indices;
	};
	
	void addToMesh(Meshf &mesh, const glm::vec3 &v);
	void addTriangle(
		Meshf &mesh,
		const glm::vec3 &v1,
		const glm::vec3 &v2,
		const glm::vec3 &v3
	);	
}

namespace gfx {
	struct Vao {
		unsigned int vertcount;
		unsigned int vaoid;
		std::vector<unsigned int> buffers;
		void bind();
	};
	//Create a quad vao (only position vectors - no texture/normal data)
	Vao createQuadVao();
	void destroyVao(Vao &vao);

	//Outputs opengl errors
	void outputErrors();
	//Converts channels to image format
	//(1 = RED, 3 = RGB, 4 = RGBA)
	GLenum getFormat(int channels); 
	//Loads a texture from 'path' and passes its data to textureid
	//returns true if texture is successfully read, false otherwise
	bool loadTexture(const char *path, unsigned int textureid);
}
