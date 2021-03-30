#include "RenderData.hpp"

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
	glVertexArrayAttribFormat(vao, 2, 1, GL_UNSIGNED_INT, GL_FALSE, offsetof(Vertex, tex_index));

	size = static_cast<GLsizei>(indices.size());
}

Mesh::~Mesh()
{
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo_vertices);
	glDeleteVertexArrays(1, &vao);
}

Mesh::Mesh(Mesh&& o) noexcept
	: vao(o.vao), vbo_vertices(o.vbo_vertices), ebo(o.ebo), size(o.size)
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

	{
		GLint vbo_size = 0;
		glGetNamedBufferParameteriv(other.vbo_vertices, GL_BUFFER_SIZE, &vbo_size);

		vertices.resize(static_cast<size_t>(vbo_size));

		indices.resize(static_cast<size_t>(other.size));

		GLvoid* buffer_ptr = glMapNamedBuffer(other.vbo_vertices, GL_READ_ONLY);
		memcpy(vertices.data(), buffer_ptr, vertices.size());
		glUnmapNamedBuffer(other.vbo_vertices);

		buffer_ptr = glMapNamedBuffer(other.ebo, GL_READ_ONLY);
		memcpy(indices.data(), buffer_ptr, indices.size());
		glUnmapNamedBuffer(other.ebo);
	}

	*this = Mesh(vertices, indices);
}

Mesh& Mesh::operator=(Mesh& other)
{
	if (&other == this)
	{
		return *this;
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	{
		GLint vbo_size = 0;
		glGetNamedBufferParameteriv(other.vbo_vertices, GL_BUFFER_SIZE, &vbo_size);

		vertices.resize(static_cast<size_t>(vbo_size));

		indices.resize(static_cast<size_t>(other.size));

		GLvoid* buffer_ptr = glMapNamedBuffer(other.vbo_vertices, GL_READ_ONLY);
		memcpy(vertices.data(), buffer_ptr, vertices.size());
		glUnmapNamedBuffer(other.vbo_vertices);

		buffer_ptr = glMapNamedBuffer(other.ebo, GL_READ_ONLY);
		memcpy(indices.data(), buffer_ptr, indices.size());
		glUnmapNamedBuffer(other.ebo);
	}

	Mesh temp_mesh{ vertices, indices };
	*this = std::move(temp_mesh);

	return *this;
}