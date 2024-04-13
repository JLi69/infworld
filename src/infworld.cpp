#include "infworld.hpp"
#include <random>
#include <glad/glad.h>

namespace mesh {
	void addToMesh(Meshf &mesh, const glm::vec3 &v)
	{
		mesh.vertices.push_back(v.x);
		mesh.vertices.push_back(v.y);
		mesh.vertices.push_back(v.z);
	}

	void addTriangle(
		Meshf &mesh,
		const glm::vec3 &v1,
		const glm::vec3 &v2,
		const glm::vec3 &v3
	) {
		glm::vec3 normal = glm::normalize(glm::cross(v1 - v3, v2 - v3));
		addToMesh(mesh, v1);
		addToMesh(mesh, normal);
		addToMesh(mesh, v2);
		addToMesh(mesh, normal);
		addToMesh(mesh, v3);
		addToMesh(mesh, normal);
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
		const mesh::ElementArrayBuffer &chunkmesh,
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
			3,
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
			3,
			GL_FLOAT, 
			false,
			CHUNK_VERT_SZ_BYTES,
			(void*)(sizeof(float) * 3)
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

namespace infworld {
	worldseed makePermutations(int seed, unsigned int count)
	{
		worldseed permutations(count);
		std::minstd_rand lcg(seed);

		for(int i = 0; i < count; i++)
			rng::createPermutation(permutations[i], lcg());

		return permutations;
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

		height += 0.1f;
		if(height < 0.1f && height >= 0.0f)
			height = 2.0f * height * height + 0.005f;
		else if(height < 0.3f && height >= 0.1f)
			height = (height - 0.1f) * 0.09f / 0.2f + 0.025f;
		else if(height >= 0.3f)
			height = (height - 0.3f) * 0.88f / 0.7f + 0.115f;

		return height; //normalized to be between -1.0 and 1.0
	}

	glm::vec3 getTerrainVertex(
		float x,
		float z,
		const worldseed &permutations,
		float maxheight
	) {
		return glm::vec3(x, getHeight(x, z, permutations) * maxheight, z);
	}	

	mesh::ElementArrayBuffer createChunkElementArray(
		const worldseed &permutations,
		int chunkx,
		int chunkz,
		float maxheight
	) {
		mesh::ElementArrayBuffer worldarraybuffer;

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

				mesh::addToMesh(worldarraybuffer.mesh, vertex);
				mesh::addToMesh(worldarraybuffer.mesh, norm);
			}
		}

		//Indices
		for(unsigned int i = 0; i < PREC; i++) {
			for(unsigned int j = 0; j < PREC; j++) {
				unsigned int index = i * (PREC + 1) + j;
				worldarraybuffer.indices.push_back(index);
				worldarraybuffer.indices.push_back(index + 1);
				worldarraybuffer.indices.push_back(index + (PREC + 1));

				worldarraybuffer.indices.push_back(index + (PREC + 1) + 1);
				worldarraybuffer.indices.push_back(index + (PREC + 1));
				worldarraybuffer.indices.push_back(index + 1);
			}
		}

		return worldarraybuffer;
	}
}
