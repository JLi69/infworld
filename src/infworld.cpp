#include "infworld.hpp"
#include <random>
#include <glad/glad.h>

constexpr unsigned int PREC = 16;
constexpr float CHUNK_SZ = 8.0f;
constexpr float FREQUENCY = 64.0f;
constexpr size_t CHUNK_VERT_SZ = 6;
constexpr size_t CHUNK_VERT_SZ_BYTES = CHUNK_VERT_SZ * sizeof(float);
constexpr unsigned int CHUNK_VERT_COUNT = PREC * PREC * 6;
//2 buffers per chunk:
//0 -> position
//1 -> normals
//2 -> indices
constexpr unsigned int BUFFER_PER_CHUNK = 3;

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

	void ChunkVaoTable::addChunk(unsigned int index, const mesh::ElementArrayBuffer &chunkmesh)
	{
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

	void ChunkVaoTable::bindVao(unsigned int index)
	{
		glBindVertexArray(vaoids.at(index));
	}

	void ChunkVaoTable::drawVao(unsigned int index)
	{	
		glDrawElements(GL_TRIANGLES, CHUNK_VERT_COUNT, GL_UNSIGNED_INT, 0);
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

		for(unsigned int i = 0; i < PREC; i++) {
			for(unsigned int j = 0; j < PREC; j++) {
				float x = -CHUNK_SZ + float(i) / float(PREC) * CHUNK_SZ * 2.0f;
				float z = -CHUNK_SZ + float(j) / float(PREC) * CHUNK_SZ * 2.0f;
				float tx = x + float(chunkx) * CHUNK_SZ * 2.0f;
				float tz = z + float(chunkz) * CHUNK_SZ * 2.0f;
				float step = 1.0f / float(PREC) * CHUNK_SZ * 2.0f;
			
				glm::vec3
					lowerleft = getTerrainVertex(tx, tz, permutations, maxheight),
					lowerright = getTerrainVertex(tx + step, tz, permutations, maxheight),
					upperleft = getTerrainVertex(tx, tz + step, permutations, maxheight),
					upperright = getTerrainVertex(tx + step, tz + step, permutations, maxheight);
				//Triangle 1	
				mesh::addTriangle(worldmesh, upperleft, lowerright, lowerleft);
				//Triangle 2	
				mesh::addTriangle(worldmesh, upperright, lowerright, upperleft);	
			}
		}	
	
		return worldmesh;
	}

	mesh::ElementArrayBuffer createChunkElementArray(
		const worldseed &permutations,
		int chunkx,
		int chunkz,
		float maxheight
	) {
		mesh::ElementArrayBuffer worldarraybuffer;

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
					norm = glm::normalize(glm::cross(v1 - vertex, v2 - vertex));

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
