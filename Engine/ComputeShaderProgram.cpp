#include "ComputeShaderProgram.hpp"

#include <stdexcept>

ComputeShaderProgram::ComputeShaderProgram(std::string_view compute_shader_code)
{
	//create our compute shader and load our code
	GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);

	{
		const char* compute_shader_source = compute_shader_code.data();

		glShaderSource(compute_shader, 1, reinterpret_cast<const GLchar* const*>(&compute_shader_source), nullptr);

		glCompileShader(compute_shader);
	}

	//check if the compilation worked
	{
		GLint shader_compiled = GL_FALSE;
		glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &shader_compiled);
		if (shader_compiled != GL_TRUE)
		{
			GLint log_size;
			glGetShaderiv(compute_shader, GL_INFO_LOG_LENGTH, &log_size);
			std::string log;
			log.resize(static_cast<size_t>(log_size) + 1);
			glGetShaderInfoLog(compute_shader, static_cast<GLsizei>(log_size) + 1, nullptr, log.data());

			throw std::runtime_error("Failed to compile compute shader: " + log);
		}
	}

	//create our program
	program = glCreateProgram();

	//attach shaders
	glAttachShader(program, compute_shader);

	//link our program, and check for errors
	glLinkProgram(program);

	{
		GLint program_compiled = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &program_compiled);
		if (program_compiled != GL_TRUE)
		{
			throw std::runtime_error("Failed to link compute shader program");
		}
	}

	//destroy our compute shader as it has been linked into our program
	glDeleteShader(compute_shader);
}

ComputeShaderProgram::ComputeShaderProgram(ComputeShaderProgram&& o) noexcept
	: program(o.program)
{
	o.program = 0;
}

ComputeShaderProgram& ComputeShaderProgram::operator=(ComputeShaderProgram&& o) noexcept
{
	if (&o == this)
	{
		return *this;
	}

	program = o.program;

	o.program = 0;

	return *this;
}

ComputeShaderProgram::~ComputeShaderProgram()
{
	glDeleteProgram(program);
}