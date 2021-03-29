#ifndef RENDER_DATA_OPENGL_HPP
#define RENDER_DATA_OPENGL_HPP

#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 tex_coord;
	uint32_t tex_index;

	inline bool operator==(const Vertex& other) const
	{
		return pos == other.pos && tex_coord == other.tex_coord && tex_index == other.tex_index;
	}
};

namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec2>()(vertex.tex_coord) << 1)) >> 1) ^
				(hash<uint32_t>()(vertex.tex_index) << 1);
		}
	};
}

class Mesh
{
public:
	GLuint vao, ebo;
	//first VBO is vertices + texture coords + texture indices
	GLuint vbo_vertices;
	GLsizei size;

	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
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

	explicit Mesh() = default;

	~Mesh()
	{
		glDeleteBuffers(1, &ebo);
		glDeleteBuffers(1, &vbo_vertices);
		glDeleteVertexArrays(1, &vao);
	}

	explicit Mesh(Mesh&& o) noexcept
		: vao(o.vao), vbo_vertices(o.vbo_vertices), ebo(o.ebo), size(o.size)
	{
		o.vao = 0;
		o.vbo_vertices = 0;
		o.ebo = 0;
		o.size = 0;
	}

	Mesh& operator=(Mesh&& o) noexcept
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

	Mesh(Mesh&) = delete;

	Mesh& operator=(Mesh&) = delete;
};

#endif