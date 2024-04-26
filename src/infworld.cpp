#include "infworld.hpp"
#include <random>
#include <glad/glad.h>
#include <chrono>

namespace infworld {
	worldseed makePermutations(int seed, unsigned int count)
	{
		worldseed permutations(count);
		std::minstd_rand lcg(seed);

		for(int i = 0; i < count; i++)
			rng::createPermutation(permutations[i], lcg());

		return permutations;
	}

	float smoothstep(float x)
	{
		return x * x * (3.0f - 2.0f * x);
	}

	float interpolate(float x, float lowerx, float upperx, float a, float b)
	{
		return (x - lowerx) / (upperx - lowerx) * (b - a) + a;
	}

	float getHeight(float x, float z, const worldseed &permutations) 
	{
		float height = 0.0f;
		float freq = FREQUENCY;
		float amplitude = 1.0f;

		for(int i = 0; i < permutations.size(); i++) {
			float h = perlin::noise(x / freq, z / freq, permutations[i]) * amplitude;
			height += h;
			freq /= 2.0f;
			amplitude /= 2.0f;
		}

		if(height < -0.1f)
			height = interpolate(height, -1.0f, -0.1f, -1.0f, 0.003f);
		else if(height >= -0.1f && height < 0.0f)
			height = interpolate(height, -0.1f, 0.0f, 0.003f, 0.03f);
		else if(height >= 0.0f && height < 0.15f)
			height = interpolate(height, 0.0f, 0.15f, 0.03f, 0.12f);
		else if(height >= 0.1f)
			height = interpolate(height, 0.15f, 1.0f, 0.12f, 1.0f);

		return height; //normalized to be between -1.0 and 1.0
	}

	glm::vec3 getTerrainVertex(
		float x,
		float z,
		const worldseed &permutations,
		float maxheight
	) {
		float h = getHeight(x, z, permutations) * maxheight;
		if(h > -0.003f && h <= 0.0f)
			h -= 0.003f;
		else if(h < 0.003f && h >= 0.0f)
			h += 0.003f;
		return glm::vec3(x, h, z);
	}

	mesh::ElementArrayBuffer<float> createChunkElementArray(
		const worldseed &permutations,
		int chunkx,
		int chunkz,
		float maxheight
	) {
		mesh::ElementArrayBuffer<float> worldarraybuffer;

		worldarraybuffer.mesh.vertices.reserve(PREC * PREC * 3 * 2);
		worldarraybuffer.indices.reserve(CHUNK_VERT_COUNT);

		for(unsigned int i = 0; i <= PREC; i++) {
			for(unsigned int j = 0; j <= PREC; j++) {
				float x = -CHUNK_SZ + float(i) / float(PREC) * CHUNK_SZ * 2.0f;
				float z = -CHUNK_SZ + float(j) / float(PREC) * CHUNK_SZ * 2.0f;
				float tx = x + float(chunkx) * CHUNK_SZ * 2.0f;
				float tz = z + float(chunkz) * CHUNK_SZ * 2.0f;

				glm::vec3 vertex = getTerrainVertex(tx, tz, permutations, maxheight);

				glm::vec3
					v1 = getTerrainVertex(tx + 0.01f, tz, permutations, maxheight),
					v2 = getTerrainVertex(tx, tz + 0.01f, permutations, maxheight),
					norm = glm::normalize(glm::cross(v2 - vertex, v1 - vertex));
				glm::vec2 n = gfx::compressNormal(norm);

				worldarraybuffer.mesh.vertices.push_back(vertex.y / maxheight);	
				worldarraybuffer.mesh.vertices.push_back(n.x);
				worldarraybuffer.mesh.vertices.push_back(n.y);
			}
		}

		//Indices
		for(unsigned int i = 0; i < PREC; i++) {
			for(unsigned int j = 0; j < PREC; j++) {
				unsigned int index = i * (PREC + 1) + j;
				worldarraybuffer.indices.push_back(index + (PREC + 1));
				worldarraybuffer.indices.push_back(index + 1);
				worldarraybuffer.indices.push_back(index);

				worldarraybuffer.indices.push_back(index + 1);
				worldarraybuffer.indices.push_back(index + (PREC + 1));
				worldarraybuffer.indices.push_back(index + (PREC + 1) + 1);
			}
		}

		return worldarraybuffer;
	}

	ChunkTable buildWorld(
		unsigned int range,
		const infworld::worldseed &permutations,
		float maxheight
	) {
		auto starttime = std::chrono::steady_clock::now();
	
		ChunkTable chunks((2 * range + 1) * (2 * range + 1));
		chunks.genBuffers();
		int ind = 0;
		for(int x = -int(range); x <= int(range); x++) {
			for(int z = -int(range); z <= int(range); z++) {
				mesh::ElementArrayBuffer<float> mesh = 
					infworld::createChunkElementArray(permutations, x, z, maxheight);
				chunks.addChunk(ind, mesh, x, z);
				ind++;
			}
		}

		auto endtime = std::chrono::steady_clock::now();
		std::chrono::duration<double> duration = endtime - starttime;
		double time = duration.count();
		printf("Time to generate world: %f\n", time);
	
		return chunks;
	}

	ChunkTable::ChunkTable(unsigned int count)
	{
		vaoids = std::vector<unsigned int>(count);
		chunkpos = std::vector<infworld::ChunkPos>(count);
		bufferids = std::vector<unsigned int>(BUFFER_PER_CHUNK * count);
		chunkcount = count;
	}

	void ChunkTable::genBuffers()
	{	
		glGenVertexArrays(vaoids.size(), &vaoids[0]);	
		glGenBuffers(bufferids.size(), &bufferids[0]);
	}

	void ChunkTable::clearBuffers()
	{
		glDeleteVertexArrays(vaoids.size(), &vaoids[0]);
		glGenBuffers(bufferids.size(), &bufferids[0]);
	}

	void ChunkTable::addChunk(
		unsigned int index,
		const mesh::ElementArrayBuffer<float> &chunkmesh,
		int x,
		int z
	) {
		chunkpos.at(index) = { x, z };

		glBindVertexArray(vaoids.at(index));

		//Buffer 0 (vertex positions)
		glBindBuffer(GL_ARRAY_BUFFER, bufferids.at(index * BUFFER_PER_CHUNK));
		glBufferData(
			GL_ARRAY_BUFFER, 
			chunkmesh.mesh.vertices.size() * sizeof(float),
			&chunkmesh.mesh.vertices[0],
			GL_STATIC_DRAW
		);
		glVertexAttribPointer(
			0,
			1,
			GL_FLOAT,
			false,
			CHUNK_VERT_SZ_BYTES,
			(void*)0
		);
		glEnableVertexAttribArray(0);

		//Buffer 1 (vertex normals)
		glBindBuffer(GL_ARRAY_BUFFER, bufferids.at(index * BUFFER_PER_CHUNK + 1));
		glBufferData(
			GL_ARRAY_BUFFER,
			chunkmesh.mesh.vertices.size() * sizeof(float),
			&chunkmesh.mesh.vertices[0],
			GL_STATIC_DRAW
		);
		glVertexAttribPointer(
			1,
			2,
			GL_FLOAT, 
			false,
			CHUNK_VERT_SZ_BYTES,
			(void*)(sizeof(float))
		);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferids.at(index * BUFFER_PER_CHUNK + 2));
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			chunkmesh.indices.size() * sizeof(unsigned int),
			&chunkmesh.indices[0],
			GL_STATIC_DRAW
		);
	}

	void ChunkTable::bindVao(unsigned int index)
	{
		glBindVertexArray(vaoids.at(index));
	}

	void ChunkTable::drawVao(unsigned int index)
	{	
		glDrawElements(GL_TRIANGLES, CHUNK_VERT_COUNT, GL_UNSIGNED_INT, 0);
	}

	infworld::ChunkPos ChunkTable::getPos(unsigned int index)
	{
		return chunkpos.at(index);
	}

	unsigned int ChunkTable::count() const
	{
		return chunkcount;
	}
}
