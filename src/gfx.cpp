#include "gfx.hpp"
#include <stdio.h>
#include <stb_image/stb_image.h>
#include "app.hpp"

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
}

namespace gfx {
	void Vao::bind()
	{	
		glBindVertexArray(vaoid);
	}

	Vao createQuadVao()
	{
		const float QUAD[] = {
			1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, -1.0f,
			-1.0f, 0.0f, 1.0f,
			-1.0f, 0.0f, -1.0f,
		};
		
		const unsigned int QUAD_INDICES[] = {
			0, 1, 2,
			1, 3, 2,
		};

		Vao quadvao;
		quadvao.buffers = std::vector<unsigned int>(2);
		glGenVertexArrays(1, &quadvao.vaoid);
		glBindVertexArray(quadvao.vaoid);
		glGenBuffers(2, &quadvao.buffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, quadvao.buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD), QUAD, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadvao.buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QUAD_INDICES), QUAD_INDICES, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

		quadvao.vertcount = 6;
		return quadvao;
	}

	void destroyVao(Vao &vao) 
	{
		glDeleteVertexArrays(1, &vao.vaoid);
		glDeleteBuffers(vao.buffers.size(), &vao.buffers[0]);
		vao.vertcount = 0;
		vao.buffers.clear();
	}

	void outputErrors()
	{
		GLenum err = glGetError();
		int errorcount = 0;
		while(err != GL_NO_ERROR) {
			fprintf(stderr, "OpenGL error: %d\n", err);
			err = glGetError();
			errorcount++;
		}
	
		if(errorcount > 0) {
			fprintf(stderr, "%d error(s)\n", errorcount);
	#ifdef DISALLOW_ERRORS 
			die("OpenGL Error, killing program.");
	#endif
		}
	}

	GLenum getFormat(int channels) 
	{
		switch(channels) {
		case 1:
			return GL_RED;
		case 3:
			return GL_RGB;
		case 4:
			return GL_RGBA;
		}
		return GL_RGBA;
	}

	bool loadTexture(const char *path, unsigned int textureid)
	{
		int width, height, channels;
		unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
		bool success = false;
		if(data) {
			success = true;
			GLenum format = getFormat(channels);
			glBindTexture(GL_TEXTURE_2D, textureid);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				format,
				width,
				height,
				0,
				format,
				GL_UNSIGNED_BYTE, 
				data
			);	
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			fprintf(stderr, "Failed to open: %s\n", path);
		}

		stbi_image_free(data);
		return success;
	}
}
