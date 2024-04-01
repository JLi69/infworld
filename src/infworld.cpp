#include "infworld.hpp"
#include <random>
#include <glad/glad.h>

constexpr float STEP = 0.1f;
constexpr float CHUNK_SZ = 4.0f;
constexpr float FREQUENCY = 16.0f;
constexpr size_t CHUNK_VERT_SZ = 6;
constexpr size_t CHUNK_VERT_SZ_BYTES = CHUNK_VERT_SZ * sizeof(float);
constexpr unsigned int BUFFER_PER_CHUNK = 2;

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

	ChunkVaoTable::ChunkVaoTable(unsigned int count)
	{
		vaoids = std::vector<unsigned int>(count);
		bufferids = std::vector<unsigned int>(BUFFER_PER_CHUNK * count);
		vertexcounts = std::vector<unsigned int>(count);
	}

	void ChunkVaoTable::genBuffers()
	{	
		glGenVertexArrays(vaoids.size(), &vaoids[0]);	
		glGenBuffers(bufferids.size(), &bufferids[0]);
	}

	void ChunkVaoTable::clearBuffers()
	{
		glDeleteVertexArrays(vaoids.size(), &vaoids[0]);
		glGenBuffers(bufferids.size(), &bufferids[0]);
	}

	void ChunkVaoTable::addChunkMesh(unsigned int index, const mesh::Meshf &chunkmesh)
	{
		glBindVertexArray(vaoids.at(index));

		vertexcounts.at(index) = chunkmesh.vertices.size() / CHUNK_VERT_SZ;

		//Buffer 0 (vertex positions)
		glBindBuffer(GL_ARRAY_BUFFER, bufferids.at(index * BUFFER_PER_CHUNK));
		glBufferData(
			GL_ARRAY_BUFFER, 
			chunkmesh.vertices.size() * sizeof(float),
			&chunkmesh.vertices[0],
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
			chunkmesh.vertices.size() * sizeof(float),
			&chunkmesh.vertices[0],
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
	}

	void ChunkVaoTable::bindVao(unsigned int index)
	{
		glBindVertexArray(vaoids.at(index));
	}

	void ChunkVaoTable::drawVao(unsigned int index)
	{	
		glDrawArrays(GL_TRIANGLES, 0, vertexcounts.at(index));
	}

	unsigned int ChunkVaoTable::vaoCount()
	{
		return vaoids.size();
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
		float maxheight = 0.0f;

		for(int i = 0; i < permutations.size(); i++) {
			float h = perlin::noise(x / freq, z / freq, permutations[i]) * freq;
			maxheight += freq;
			height += h;
			freq /= 2.0f;
		}

		return height / maxheight; //normalized to be between -1.0 and 1.0
	}

	glm::vec3 getTerrainVertex(
		float x,
		float z,
		const worldseed &permutations,
		float maxheight
	) {
		return glm::vec3(x, getHeight(x, z, permutations) * maxheight, z);
	}

	mesh::Meshf createChunkMesh(
		const worldseed &permutations,
		int chunkx,
		int chunkz,
		float maxheight
	) {
		mesh::Meshf worldmesh;
	
		float x = -CHUNK_SZ;
		while(x < CHUNK_SZ) {
			float z = -CHUNK_SZ;
			while(z < CHUNK_SZ) {
				float tx = x + CHUNK_SZ * 2.0f * float(chunkx);
				float tz = z + CHUNK_SZ * 2.0f * float(chunkz);

				glm::vec3
					lowerleft = getTerrainVertex(tx, tz, permutations, maxheight),
					lowerright = getTerrainVertex(tx + STEP, tz, permutations, maxheight),
					upperleft = getTerrainVertex(tx, tz + STEP, permutations, maxheight),
					upperright = getTerrainVertex(tx + STEP, tz + STEP, permutations, maxheight);
				//Triangle 1	
				mesh::addTriangle(worldmesh, upperleft, lowerright, lowerleft);
				//Triangle 2	
				mesh::addTriangle(worldmesh, upperright, lowerright, upperleft);	
				z += STEP;
			}
			x += STEP;
		}
	
		return worldmesh;
	}
}
