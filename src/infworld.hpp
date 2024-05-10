#pragma once
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include "noise.hpp"
#include "gfx.hpp"

constexpr unsigned int PREC = 32;
constexpr float CHUNK_SZ = 48.0f;
constexpr float SCALE = 2.5f;
constexpr float FREQUENCY = 720.0f;
constexpr size_t CHUNK_VERT_SZ = 3;
constexpr size_t CHUNK_VERT_SZ_BYTES = CHUNK_VERT_SZ * sizeof(float);
constexpr unsigned int CHUNK_VERT_COUNT = PREC * PREC * 6;
//2 buffers per chunk:
//0 -> position
//1 -> normals
//2 -> indices
constexpr unsigned int BUFFER_PER_CHUNK = 3;

namespace infworld {
	//We will use a seed value (an integer) to generate multiple
	//pseudorandom permutations to feed into the perlin noise generator
	//for world generation
	typedef std::vector<rng::permutation256> worldseed;

	struct ChunkPos {
		int x = 0, z = 0;
	};

	struct ChunkData {
		mesh::ElementArrayBuffer<float> chunkmesh;
		ChunkPos position;
	};

	class ChunkTable {
		unsigned int chunkcount;
		unsigned int size;
		float chunkscale;
		float height;
		std::vector<unsigned int> vaoids;
		std::vector<unsigned int> bufferids; 
		std::vector<ChunkPos> chunkpos;
		int centerx = 0, centerz = 0;

		//For generating new chunks
		std::vector<unsigned int> indices;
		std::vector<ChunkPos> newChunks;
	public:
		ChunkTable(unsigned int range, float scale, float h);
		void genBuffers();
		void clearBuffers();
		void addChunk(
			unsigned int index,
			const mesh::ElementArrayBuffer<float> &chunkmesh,
			int x,
			int z
		);
		void addChunk(unsigned int index, const ChunkData &chunk);
		void bindVao(unsigned int index);
		ChunkPos getPos(unsigned int index);
		unsigned int count() const;
		ChunkPos getCenter();
		void setCenter(int x, int z);
		void generateNewChunks(
			float camerax,
			float cameraz,
			const worldseed &permutations
		);
	};

	worldseed makePermutations(int seed, unsigned int count);
	float getHeight(float x, float z, const worldseed &permutations);
	float interpolate(float x, float lowerx, float upperx, float a, float b);
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
		float maxheight,
		float chunkscale
	);
	ChunkData buildChunk(
		const infworld::worldseed &permutations,
		int x,
		int z,
		float maxheight,
		float chunkscale
	);
	ChunkTable buildWorld(
		unsigned int range,
		const infworld::worldseed &permutations,
		float maxheight,
		float chunkscale
	);
}
