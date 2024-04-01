#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "perlin.hpp"

namespace mesh {
	template<typename T>
	struct Mesh {
		std::vector<T> vertices;
		size_t vertexsz;
	};

	//Mesh of floats
	typedef Mesh<float> Meshf;
	
	void addToMesh(Meshf &mesh, const glm::vec3 &v);
	void addTriangle(
		Meshf &mesh,
		const glm::vec3 &v1,
		const glm::vec3 &v2,
		const glm::vec3 &v3
	);

	class ChunkVaoTable {
		std::vector<unsigned int> vaoids;
		std::vector<unsigned int> vertexcounts;
		std::vector<unsigned int> bufferids;
	public:
		ChunkVaoTable(unsigned int count);
		void genBuffers();
		void clearBuffers();
		void addChunkMesh(unsigned int index, const mesh::Meshf &chunkmesh);
		void bindVao(unsigned int index);
		void drawVao(unsigned int index);
		unsigned int vaoCount();
	};
}

namespace infworld
{
	//We will use a seed value (an integer) to generate multiple
	//pseudorandom permutations to feed into the perlin noise generator
	//for world generation
	typedef std::vector<rng::permutation256> worldseed;

	worldseed makePermutations(int seed, unsigned int count);
	float getHeight(float x, float z, const worldseed &permutations);
	glm::vec3 getTerrainVertex(
		float x,
		float z,
		const worldseed &permutations,
		float maxheight
	);
	mesh::Meshf createChunkMesh(
		const worldseed &permutations,
		int chunkx,
		int chunkz,
		float maxheight
	);
}
