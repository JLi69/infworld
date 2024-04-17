#pragma once
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include "noise.hpp"
#include "gfx.hpp"

constexpr unsigned int PREC = 64;
constexpr float CHUNK_SZ = 32.0f;
constexpr float FREQUENCY = 300.0f;
constexpr size_t CHUNK_VERT_SZ = 3;
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
			const mesh::ElementArrayBuffer<float> &chunkmesh,
			int x,
			int z
		);
		void bindVao(unsigned int index);
		void drawVao(unsigned int index);
		infworld::ChunkPos getPos(unsigned int index);
		unsigned int count() const;
	};
	
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
	mesh::ElementArrayBuffer<float> createChunkElementArray(
		const worldseed &permutations,
		int chunkx,
		int chunkz,
		float maxheight
	);
	ChunkTable buildWorld(
		unsigned int range,
		const infworld::worldseed &permutations,
		float maxheight
	);
}
