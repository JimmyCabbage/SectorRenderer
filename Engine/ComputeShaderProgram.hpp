#ifndef COMPUTE_PROGRAM_OPENGL_HPP
#define COMPUTE_PROGRAM_OPENGL_HPP

#include <glad/glad.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <string_view>

class ComputeShaderProgram
{
public:
	GLuint program;

	inline void use() const { glUseProgram(program); }

	explicit ComputeShaderProgram(std::string_view compute_shader_code);

	explicit ComputeShaderProgram() = default;

	explicit ComputeShaderProgram(ComputeShaderProgram&& o) noexcept;

	ComputeShaderProgram& operator=(ComputeShaderProgram&& o) noexcept;

	~ComputeShaderProgram();
};

#endif