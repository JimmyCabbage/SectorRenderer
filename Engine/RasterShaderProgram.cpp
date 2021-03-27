#include "RasterShaderProgram.hpp"

#include <stdexcept>

RasterShaderProgram::RasterShaderProgram(std::string_view vertex_shader_code, std::string_view fragment_shader_code)
{
	//create our vertex shader and load our code
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

	{
		const char* vertex_shader_source = vertex_shader_code.data();

		glShaderSource(vertex_shader, 1, reinterpret_cast<const GLchar* const*>(&vertex_shader_source), nullptr);

		glCompileShader(vertex_shader);
	}

	//check if the compilation worked
	{
		GLint shader_compiled = GL_FALSE;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_compiled);
		if (shader_compiled != GL_TRUE)
		{
			throw std::runtime_error("Failed to compile vertex shader");
		}
	}

	//same with the fragment shader
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	{
		const char* fragment_shader_source = fragment_shader_code.data();

		glShaderSource(fragment_shader, 1, reinterpret_cast<const char* const*>(&fragment_shader_source), nullptr);

		glCompileShader(fragment_shader);
	}

	//check compilation
	{
		GLint shader_compiled = GL_FALSE;
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_compiled);
		if (shader_compiled != GL_TRUE)
		{
			throw std::runtime_error("Failed to compile fragment shader");
		}
	}

	//create our program
	program = glCreateProgram();

	//attach shaders
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	//link our program, and check for errors
	glLinkProgram(program);

	{
		GLint program_compiled = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &program_compiled);
		if (program_compiled != GL_TRUE)
		{
			throw std::runtime_error("Failed to link shader program");
		}
	}

	//destroy our shaders as they have been linked in
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

RasterShaderProgram::~RasterShaderProgram()
{
	glDeleteProgram(program);
}