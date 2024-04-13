#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "noise.hpp"

constexpr unsigned int PREC = 64;
constexpr float CHUNK_SZ = 32.0f;
constexpr float FREQUENCY = 300.0f;
constexpr size_t CHUNK_VERT_SZ = 6;
constexpr size_t CHUNK_VERT_SZ_BYTES = CHUNK_VERT_SZ * sizeof(float);
constexpr unsigned int CHUNK_VERT_COUNT = PREC * PREC * 6;
//2 buffers per chunk:
//0 -> position
//1 -> normals
//2 -> indices
constexpr unsigned int BUFFER_PER_CHUNK = 3;

namespace infworld {
	struct ChunkPos {
		int x = 0, z = 0;
	};
}

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

	class ChunkTable {
		unsigned int chunkcount;
		std::vector<unsigned int> vaoids;
		std::vector<unsigned int> bufferids;
		std::vector<infworld::ChunkPos> chunkpos;
	public:
		ChunkTable(unsigned int count);
		void genBuffers();
		void clearBuffers();
		void addChunk(
			unsigned int index,
			const mesh::ElementArrayBuffer &chunkmesh,
			int x,
			int z
		);
		void bindVao(unsigned int index);
		void drawVao(unsigned int index);
		infworld::ChunkPos getPos(unsigned int index);
		unsigned int count() const;
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
	mesh::ElementArrayBuffer createChunkElementArray(
		const worldseed &permutations,
		int chunkx,
		int chunkz,
		float maxheight
	);
}
