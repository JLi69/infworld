#define _USE_MATH_DEFINES
#include <math.h>
#include "gfx.hpp"
#include <stdio.h>
#include <stb_image/stb_image.h>
#include <assert.h>
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
	void Vao::bind() const
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

	Vao createCubeVao()
	{
		const float CUBE[] = {
			1.0f, 1.0f, 1.0f, //0
			-1.0f, 1.0f, 1.0f, //1
			-1.0f, -1.0f, 1.0f, //2
			1.0f, -1.0f, 1.0f, //3
			1.0f, 1.0f, -1.0f, //4
			-1.0f, 1.0f, -1.0f, //5
			-1.0f, -1.0f, -1.0f, //6
			1.0f, -1.0f, -1.0f, //7
		};

		const unsigned int CUBE_INDICES[] = {
			7, 6, 5,
    		5, 4, 7,

    		5, 6, 2,
    		2, 1, 5,

    		0, 3, 7,
    		7, 4, 0,

    		0, 1, 2,
    		2, 3, 0,

    		0, 4, 5,
			5, 1, 0,

    		7, 2, 6,
    		3, 2, 7,
		};

		Vao cubevao;
		cubevao.buffers = std::vector<unsigned int>(2);
		glGenVertexArrays(1, &cubevao.vaoid);
		glBindVertexArray(cubevao.vaoid);
		glGenBuffers(2, &cubevao.buffers[0]);
		glBindBuffer(GL_ARRAY_BUFFER, cubevao.buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE), CUBE, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubevao.buffers[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CUBE_INDICES), CUBE_INDICES, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		cubevao.vertcount = 36;
		return cubevao;
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
		else
			fprintf(stderr, "Failed to open: %s\n", path);

		stbi_image_free(data);
		return success;
	}

	bool loadCubemap(const std::vector<std::string> &faces, unsigned int textureid)
	{
		bool success = true;
		int width, height, channels;
		assert(faces.size() == 6); //faces must have 6 elements in it
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureid);

		for(int i = 0; i < 6; i++) {
			unsigned char* data = 
				stbi_load(faces.at(i).c_str(), &width, &height, &channels, 0);

			if(data) {	
				GLenum format = getFormat(channels);
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					format,
					width,
					height,
					0,
					format,
					GL_UNSIGNED_BYTE, 
					data
				);
			}
			else {
				fprintf(stderr, "Failed to open cubemap file: %s\n", faces.at(i).c_str()); 
				success = false;
			}

			stbi_image_free(data);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return success;
	}

	float getAngle(float x, float y)
	{
		if(x == 0.0f && y > 0.0f)
			return M_PI / 2.0f;
		if(x == 0.0f && y < 0.0f)
			return M_PI / 2.0f * 3.0f;

		float angle = atanf(y / x);

		if(x > 0.0f && y < 0.0f)
			angle += M_PI * 2.0f;
		else if(x < 0.0f && y > 0.0f)
			angle += M_PI;
		else if(x < 0.0f && y < 0.0f)
			angle += M_PI;

		return angle;
	}

	glm::vec2 compressNormal(glm::vec3 n)
	{
		return glm::vec2(getAngle(n.x, n.z), asinf(n.y));
	}
}
