#pragma once
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <string>

namespace mesh {
	template<typename T>
	struct Mesh {
		std::vector<T> vertices;
	};

	//Mesh of floats
	typedef Mesh<float> Meshf;

	template<typename T>
	struct ElementArrayBuffer {
		Mesh<T> mesh;
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
	//Create a cube vao (only position vectors - not texture/normal data)
	Vao createCubeVao();
	void destroyVao(Vao &vao);

	//Outputs opengl errors
	void outputErrors();
	//Converts channels to image format
	//(1 = RED, 3 = RGB, 4 = RGBA)
	GLenum getFormat(int channels); 
	//Loads a texture from 'path' and passes its data to textureid
	//returns true if texture is successfully read, false otherwise
	bool loadTexture(const char *path, unsigned int textureid);
	//Attempts to load all 6 faces of a cubemap from a vector of 6 paths
	//If loading a face fails, the function will return false, otherwise true
	bool loadCubemap(const std::vector<std::string> &faces, unsigned int textureid);

	//Converts a normal vector (x, y, z) to a 2d vector consisting of
	//angles that represent the vector (we assume the original 3d vector
	//has magnitude of 1). This can allow for a smaller amount of data to
	//be used in the mesh and improve performance
	glm::vec2 compressNormal(glm::vec3 n);
}
