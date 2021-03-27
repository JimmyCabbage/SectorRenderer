#ifndef SHADER_PROGRAM_OPENGL_HPP
#define SHADER_PROGRAM_OPENGL_HPP

#include <glad/glad.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <string_view>

class RasterShaderProgram
{
public:
	GLuint program;

	inline void use() const { glUseProgram(program); }

	explicit RasterShaderProgram(std::string_view vertex_shader_code, std::string_view fragment_shader_code);

	explicit RasterShaderProgram() = default;

	explicit RasterShaderProgram(RasterShaderProgram&& o) noexcept
		: program(o.program)
	{
		o.program = 0;
	}

	RasterShaderProgram& operator=(RasterShaderProgram&& o) noexcept
	{
		if (&o == this)
		{
			return *this;
		}

		program = o.program;

		o.program = 0;

		return *this;
	}

	~RasterShaderProgram();
};

#endif