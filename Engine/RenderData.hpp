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
	float tex_index;

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
				(hash<float>()(vertex.tex_index) << 1);
		}
	};
}

class Mesh
{
	GLuint vao, ebo;
	//first VBO is vertices + texture coords + texture indices
	GLuint vbo_vertices;
	GLsizei size;

public:
	explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	explicit Mesh() = default;

	~Mesh();

	explicit Mesh(Mesh&& o) noexcept;

	Mesh& operator=(Mesh&& o) noexcept;

	explicit Mesh(Mesh& other);

	Mesh& operator=(Mesh& other);

	void draw();
};

class TextureArray2d
{
	GLuint texture_array;

public:
	explicit TextureArray2d(const std::vector<const char*>& texture_filenames, const size_t texture_width, const size_t texture_height);

	explicit TextureArray2d() noexcept = default;

	~TextureArray2d();

	explicit TextureArray2d(TextureArray2d&& o) noexcept;

	TextureArray2d& operator=(TextureArray2d&& o) noexcept;

	explicit TextureArray2d(TextureArray2d&) = delete;

	TextureArray2d& operator=(TextureArray2d&) = delete;

	void bind(uint32_t texture_unit);
};

#endif