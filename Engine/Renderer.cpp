#include "Renderer.hpp"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

#ifndef NDEBUG
void APIENTRY opengl_debug_output(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cerr << "---------------" << '\n';
	std::cerr << "Debug message (" << id << "): " << message << '\n';

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other"; break;
	} std::cerr << '\n';

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other"; break;
	} std::cerr << '\n';

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification"; break;
	} std::cerr << '\n';
	std::cerr << '\n';
}
#endif

Renderer::Renderer()
{
	//initialize our Window and OpenGL
	init_window_renderer();

	set_sdl_settings();

	set_opengl_settings();

	//initialize our objects
	init_game_objects();

	is_running = true;
}

Renderer::~Renderer()
{
	destroy_window_renderer();
}

void Renderer::run()
{
	while (is_running)
	{
		//calculate delta time
		prev_time = current_time;
		current_time = SDL_GetPerformanceCounter();
		delta_time = ((current_time - prev_time) / (float)SDL_GetPerformanceFrequency());

		//sdl events
		get_events();

		handle_events();

		//draw frame
		draw();
	}
}

void Renderer::get_events()
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_WINDOWEVENT:
			switch (ev.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
				glViewport(0, 0, ev.window.data1, ev.window.data2);
				break;
			}
			break;
		case SDL_MOUSEMOTION:
			camera.ProcessMouseMovement((float)ev.motion.xrel, -(float)ev.motion.yrel);
			break;
		case SDL_KEYDOWN:
			switch (ev.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				is_running = false;
				break;
			}
			[[fallthrough]];
		case SDL_KEYUP:
			switch (ev.key.keysym.sym)
			{
			case SDLK_w:
				wasd[0] = SDL_KEYDOWN == ev.type;
				break;
			case SDLK_a:
				wasd[1] = SDL_KEYDOWN == ev.type;
				break;
			case SDLK_s:
				wasd[2] = SDL_KEYDOWN == ev.type;
				break;
			case SDLK_d:
				wasd[3] = SDL_KEYDOWN == ev.type;
				break;
			}
			break;
		}
	}
}

void Renderer::handle_events()
{
	if (wasd[0])
	{
		camera.ProcessKeyboard(cam::Movement::Forward, delta_time);
	}
	if (wasd[1])
	{
		camera.ProcessKeyboard(cam::Movement::Left, delta_time);
	}
	if (wasd[2])
	{
		camera.ProcessKeyboard(cam::Movement::Backward, delta_time);
	}
	if (wasd[3])
	{
		camera.ProcessKeyboard(cam::Movement::Right, delta_time);
	}
}

void Renderer::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.4f, 100.0f);

	const auto pv = projection * camera.GetViewMatrix();

	main_shader.use();

	glUniformMatrix4fv(glGetUniformLocation(main_shader.program, "pv"), 1, GL_FALSE, glm::value_ptr(pv));

	glBindVertexArray(batch_mesh.vao);
	glDrawElements(GL_TRIANGLES, batch_mesh.ebo, GL_UNSIGNED_INT, nullptr);

	SDL_GL_SwapWindow(window);
}

void Renderer::init_window_renderer()
{
	//initialize our window
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		throw std::runtime_error("Failed to initialize SDL");
	}

	window = SDL_CreateWindow("BSP Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (nullptr == window)
	{
		throw std::runtime_error("Failed to create window");
	}

	//set opengl settings, ask for an OpenGL 4.3 Core context, and create a context
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifndef NDEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	context = SDL_GL_CreateContext(window);
	if (nullptr == context)
	{
		throw std::runtime_error("Failed to create OpenGL context");
	}

	//load our OpenGL functions pointers
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		throw std::runtime_error("Failed to load GLAD");
	}
}

void Renderer::set_sdl_settings()
{
	wasd = {};

	SDL_GL_SetSwapInterval(0);

	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Renderer::set_opengl_settings()
{
	glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(opengl_debug_output, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif

	//enable depth testing
	glEnable(GL_DEPTH_TEST);

	main_shader = RasterShaderProgram
	{
		"#version 430\n"
		"uniform mat4 pv;"
		"layout(location = 0) in vec3 inPos;"
		"layout(location = 1) in vec2 inTextureCoord;"
		"layout(location = 2) in uint inTextureIndex;"
		"layout(location = 1) out uint outTextureIndex;"
		"layout(location = 0) out vec3 outColor;"
		"void main() { gl_Position = pv * vec4( inPos, 1.0 ); outColor = vec3(inTextureCoord, 0.0f); outTextureIndex = inTextureIndex; }",
		"#version 430\nlayout(location = 0) in vec3 inColor; layout(location = 0) out vec4 outColor; void main() { outColor = vec4( inColor, 1.0 ); }"
	};
}

void Renderer::init_game_objects()
{
	/*
	//load map
	std::vector<Vertex> vertices;
	std::vector<uint32_t> texture_indices;
	std::vector<uint32_t> indices;

	{
		std::vector<Sector> sectors;
		std::vector<glm::vec2> vertices;
		{
			std::ifstream map_file{ "map.sec" };

			//read file line by line and process
			{
				std::string line;
				while (std::getline(map_file, line))
				{
					std::stringstream line_stream{ line };

					std::string prefix_identifier;
					line_stream >> prefix_identifier;

					if (prefix_identifier.compare("vertex") == 0)
					{
						glm::vec2 vec{ 0.0f, 0.0f };

						line_stream >> vec.x;

						while (line_stream >> vec.y)
						{
							vertices.push_back(vec);
						}
					}
					else if (prefix_identifier.compare("sector") == 0)
					{
						Sector sector;

						line_stream >> sector.ceil >> sector.floor;

						std::vector<uint32_t> integers;
						{
							int64_t get_integer;
							while (line_stream >> get_integer)
							{
								if (get_integer < 0)
								{
									const uint32_t integer = static_cast<uint32_t>(get_integer) | (1UL << 31);

									integers.push_back(integer);
								}
								else
								{
									const uint32_t integer = static_cast<uint32_t>(get_integer);

									integers.push_back(integer);
								}
							}
						}

						if (integers.size() < 6)
						{
							throw std::logic_error("Sector size must have at least 3 vertices & neighbors");
						}

						const size_t size = integers.size() / 2;

						for (size_t i = 0; i < size; i++)
						{
							sector.vertices.push_back(integers[i]);
							sector.neighbors.push_back(integers[i + size]);
						}

						sectors.push_back(sector);
					}
				}
			}
			map_file.close();
		}

		//lambda to add to indices and vertices easier

		//change 2d sectors into 3d data
		for (auto& sector : sectors)
		{
			//
		}
	}
	*/

	const std::vector<Vertex> vertices
	{
		Vertex{ glm::vec3{-7.5f, -7.5f, 0.0f }, glm::vec2{ 0.0f, 0.0f } },
		Vertex{ glm::vec3{ 7.5f, -7.5f, 0.0f }, glm::vec2{ 1.0f, 0.0f } },
		Vertex{ glm::vec3{ 0.0f,  7.5f, 0.0f }, glm::vec2{ 0.5f, 1.0f } }
	};

	const std::vector<uint32_t> texture_indices
	{
		0,
		0,
		0
	};

	const std::vector<unsigned int> indices
	{
		0, 1, 2
	};

	batch_mesh = Mesh(vertices, texture_indices, indices);
}

void Renderer::destroy_window_renderer()
{
	SDL_GL_DeleteContext(context);

	SDL_DestroyWindow(window);

	SDL_Quit();
}