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

	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	explicit Mesh() = default;

	~Mesh();

	explicit Mesh(Mesh&& o) noexcept;

	Mesh& operator=(Mesh&& o) noexcept;

	explicit Mesh(Mesh& other);

	Mesh& operator=(Mesh& other);
};

#endif