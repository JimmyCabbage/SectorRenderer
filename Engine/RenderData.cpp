#include "RenderData.hpp"

#include <stdexcept>
#include <string>

#include "stb_image.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo_vertices);
	glCreateBuffers(1, &ebo);

	glVertexArrayElementBuffer(vao, ebo);
	glNamedBufferData(ebo, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	glVertexArrayVertexBuffer(vao, 0, vbo_vertices, 0, sizeof(Vertex));
	glNamedBufferData(vbo_vertices, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glVertexArrayAttribBinding(vao, 0, 0);
	glEnableVertexArrayAttrib(vao, 0);
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));

	glVertexArrayAttribBinding(vao, 1, 0);
	glEnableVertexArrayAttrib(vao, 1);
	glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coord));

	glVertexArrayAttribBinding(vao, 2, 0);
	glEnableVertexArrayAttrib(vao, 2);
	glVertexArrayAttribFormat(vao, 2, 1, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_index));

	size = static_cast<GLsizei>(indices.size());
}

Mesh::~Mesh()
{
	if (vao)
	{
		glDeleteBuffers(1, &ebo);
		glDeleteBuffers(1, &vbo_vertices);
		glDeleteVertexArrays(1, &vao);
	}
}

Mesh::Mesh(Mesh&& o) noexcept
	: vao(o.vao), ebo(o.ebo), vbo_vertices(o.vbo_vertices), size(o.size)
{
	o.vao = 0;
	o.vbo_vertices = 0;
	o.ebo = 0;
	o.size = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept
{
	if (&o == this)
	{
		return *this;
	}

	vao = o.vao;
	vbo_vertices = o.vbo_vertices;
	ebo = o.ebo;
	size = o.size;

	o.vao = 0;
	o.vbo_vertices = 0;
	o.ebo = 0;
	o.size = 0;

	return *this;
}

Mesh::Mesh(Mesh& other)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	if (other.vao)
	{
		GLint vbo_size = 0;
		glGetNamedBufferParameteriv(other.vbo_vertices, GL_BUFFER_SIZE, &vbo_size);

		vertices.resize(static_cast<size_t>(vbo_size));

		GLvoid* buffer_ptr = glMapNamedBuffer(other.vbo_vertices, GL_READ_ONLY);
		memcpy(vertices.data(), buffer_ptr, vertices.size());
		glUnmapNamedBuffer(other.vbo_vertices);

		indices.resize(static_cast<size_t>(other.size));

		buffer_ptr = glMapNamedBuffer(other.ebo, GL_READ_ONLY);
		memcpy(indices.data(), buffer_ptr, indices.size());
		glUnmapNamedBuffer(other.ebo);

		//create objects using data
		glCreateVertexArrays(1, &vao);
		glCreateBuffers(1, &vbo_vertices);
		glCreateBuffers(1, &ebo);

		glVertexArrayElementBuffer(vao, ebo);
		glNamedBufferData(ebo, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

		glVertexArrayVertexBuffer(vao, 0, vbo_vertices, 0, sizeof(Vertex));
		glNamedBufferData(vbo_vertices, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		glVertexArrayAttribBinding(vao, 0, 0);
		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));

		glVertexArrayAttribBinding(vao, 1, 0);
		glEnableVertexArrayAttrib(vao, 1);
		glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coord));

		glVertexArrayAttribBinding(vao, 2, 0);
		glEnableVertexArrayAttrib(vao, 2);
		glVertexArrayAttribFormat(vao, 2, 1, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_index));

		size = static_cast<GLsizei>(indices.size());
	}
	else
	{
		vao = 0;
		vbo_vertices = 0;
		ebo = 0;
		size = 0;
	}
}

Mesh& Mesh::operator=(Mesh& other)
{
	if (&other == this)
	{
		return *this;
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	if (other.vao)
	{
		GLint vbo_size = 0;
		glGetNamedBufferParameteriv(other.vbo_vertices, GL_BUFFER_SIZE, &vbo_size);

		vertices.resize(static_cast<size_t>(vbo_size));

		GLvoid* buffer_ptr = glMapNamedBuffer(other.vbo_vertices, GL_READ_ONLY);
		memcpy(vertices.data(), buffer_ptr, vertices.size());
		glUnmapNamedBuffer(other.vbo_vertices);

		indices.resize(static_cast<size_t>(other.size));

		buffer_ptr = glMapNamedBuffer(other.ebo, GL_READ_ONLY);
		memcpy(indices.data(), buffer_ptr, indices.size());
		glUnmapNamedBuffer(other.ebo);

		//create objects using data
		glCreateVertexArrays(1, &vao);
		glCreateBuffers(1, &vbo_vertices);
		glCreateBuffers(1, &ebo);

		glVertexArrayElementBuffer(vao, ebo);
		glNamedBufferData(ebo, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

		glVertexArrayVertexBuffer(vao, 0, vbo_vertices, 0, sizeof(Vertex));
		glNamedBufferData(vbo_vertices, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		glVertexArrayAttribBinding(vao, 0, 0);
		glEnableVertexArrayAttrib(vao, 0);
		glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));

		glVertexArrayAttribBinding(vao, 1, 0);
		glEnableVertexArrayAttrib(vao, 1);
		glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_coord));

		glVertexArrayAttribBinding(vao, 2, 0);
		glEnableVertexArrayAttrib(vao, 2);
		glVertexArrayAttribFormat(vao, 2, 1, GL_FLOAT, GL_FALSE, offsetof(Vertex, tex_index));

		size = static_cast<GLsizei>(indices.size());
	}
	else
	{
		vao = 0;
		vbo_vertices = 0;
		ebo = 0;
		size = 0;
	}

	return *this;
}

void Mesh::draw()
{
	if (vao)
	{
		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		throw std::runtime_error("Tried to draw with blank VAO");
	}
}

TextureArray2d::TextureArray2d(const std::vector<const char*>& texture_filenames, const size_t texture_width, const size_t texture_height)
{
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture_array);

	//allocate storage
	glTextureStorage3D(texture_array, 4, GL_RGB8, static_cast<GLsizei>(texture_width), static_cast<GLsizei>(texture_height), static_cast<GLsizei>(texture_filenames.size()));

	for (size_t i = 0; i < texture_filenames.size(); i++)
	{
		int width, height, nr_channels;
		unsigned char* texture = stbi_load(texture_filenames[i], &width, &height, &nr_channels, 3);

		if (width != static_cast<int>(texture_width) || height != static_cast<int>(texture_height))
		{
			throw std::runtime_error("texture loaded is not of size (" + std::to_string(texture_width) + " x " + std::to_string(texture_height) + ")");
		}

		if (nullptr == texture)
		{
			throw std::runtime_error("Failed to load texture from file");
		}

		//store image
		glTextureSubImage3D(texture_array, 0, 0, 0, static_cast<GLint>(i), static_cast<GLsizei>(texture_width), static_cast<GLsizei>(texture_height), 1, GL_RGB, GL_UNSIGNED_BYTE, texture);

		stbi_image_free(texture);
	}

	glGenerateTextureMipmap(texture_array);

	glTextureParameteri(texture_array, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTextureParameteri(texture_array, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTextureParameteri(texture_array, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(texture_array, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

TextureArray2d::~TextureArray2d()
{
	if (texture_array)
	{
		glDeleteTextures(1, &texture_array);
	}
}

TextureArray2d::TextureArray2d(TextureArray2d&& o) noexcept
	: texture_array(o.texture_array)
{
	o.texture_array = 0;
}

TextureArray2d& TextureArray2d::operator=(TextureArray2d&& o) noexcept
{
	if (&o == this)
	{
		return *this;
	}

	texture_array = o.texture_array;

	o.texture_array = 0;

	return *this;
}

void TextureArray2d::bind(uint32_t texture_unit)
{
	if (texture_array)
	{
		glBindTextureUnit(texture_unit, texture_array);
	}
	else
	{
		throw std::runtime_error("Tried to bind invalid TextureArray2d");
	}
}