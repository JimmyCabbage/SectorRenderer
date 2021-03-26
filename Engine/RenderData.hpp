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

	inline bool operator==(const Vertex& other) const
	{
		return pos == other.pos && tex_coord == other.tex_coord;
	}
};

namespace std
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec2>()(vertex.tex_coord) << 1)) >> 1);
		}
	};
}

class Mesh
{
public:
	GLuint vao, vbo_vertexdata, vbo_texturedata, ebo;
	GLsizei size;

	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& textures, const std::vector<uint32_t>& indices)
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo_vertexdata);
		glGenBuffers(1, &vbo_texturedata);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertexdata);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));

		glBindBuffer(GL_ARRAY_BUFFER, vbo_texturedata);
		glBufferData(GL_ARRAY_BUFFER, textures.size() * sizeof(uint32_t), textures.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(uint32_t), (void*)0);

		size = static_cast<GLsizei>(indices.size());
	}

	explicit Mesh() = default;

	~Mesh()
	{
		glDeleteBuffers(1, &ebo);
		glDeleteBuffers(1, &vbo_vertexdata);
		glDeleteBuffers(1, &vbo_texturedata);
		glDeleteVertexArrays(1, &vao);
	}

	explicit Mesh(Mesh&& o) noexcept
		: vao(o.vao), vbo_vertexdata(o.vbo_vertexdata), vbo_texturedata(o.vbo_texturedata), ebo(o.ebo), size(o.size)
	{
		o.vao = 0;
		o.vbo_vertexdata = 0;
		o.vbo_texturedata = 0;
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
		vbo_vertexdata = o.vbo_vertexdata;
		vbo_texturedata = o.vbo_texturedata;
		ebo = o.ebo;
		size = o.size;

		o.vao = 0;
		o.vbo_vertexdata = 0;
		o.vbo_texturedata = 0;
		o.ebo = 0;
		o.size = 0;

		return *this;
	}

	Mesh(Mesh&) = delete;

	Mesh& operator=(Mesh&) = delete;
};

#endif