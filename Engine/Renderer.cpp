#include "Renderer.hpp"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"

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
		const auto current_time = SDL_GetPerformanceCounter();
		delta_time = (double)((current_time - prev_time) / (double)SDL_GetPerformanceFrequency());
		prev_time = current_time;

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
				window_width = ev.window.data1;
				window_height = ev.window.data2;
				glViewport(0, 0, window_width, window_height);
				break;
			}
			break;
		case SDL_MOUSEMOTION:
			player.mouse_move(static_cast<float>(ev.motion.xrel), -static_cast<float>(ev.motion.yrel));
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
			case SDLK_SPACE:
				//if (SDL_KEYDOWN == ev.type) player.jump(delta_time);
				break;
			}
			break;
		}
	}
}

void Renderer::handle_events()
{
	auto dir = Player::MoveDir::NONE;
	if (wasd[0])
	{
		dir = dir | Player::MoveDir::FORWARD;
	}
	if (wasd[1])
	{
		dir = dir | Player::MoveDir::LEFT;
	}
	if (wasd[2])
	{
		dir = dir | Player::MoveDir::BACKWARD;
	}
	if (wasd[3])
	{
		dir = dir | Player::MoveDir::RIGHT;
	}

	player.move(dir, delta_time);

	player.collision(sectors, delta_time);
}

void Renderer::draw()
{
	glClear(/*GL_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT);

	const glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);

	const auto pv = projection * player.get_view_matrix();

	texture_array.bind(0);

	main_shader.use();

	glProgramUniformMatrix4fv(main_shader.program, 0, 1, GL_FALSE, glm::value_ptr(pv));

	const auto view_pos = player.get_pos();
	glProgramUniform3f(main_shader.program, 1, view_pos.x, view_pos.y, view_pos.z);

	map_mesh.draw();

	SDL_GL_SwapWindow(window);
}

void Renderer::init_window_renderer()
{
	//initialize our window
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		throw std::runtime_error("Failed to initialize SDL");
	}

	window_width = 1280;
	window_height = 720;

	window = SDL_CreateWindow("Sector Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (nullptr == window)
	{
		throw std::runtime_error("Failed to create window");
	}

	//set opengl settings, ask for an OpenGL 4.3 Core context, and create a context
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
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
	glClearColor(0.0f, 0.6f, 0.6f, 1.0f);

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(opengl_debug_output, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif

	//enable depth testing
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	//create the main shader that's used
	//vertex shader
	constexpr const char* vertex_shader_code =
		"#version 430 core\n"
		"layout(location = 0) uniform mat4 pv;"
		"layout(location = 0) in vec3 inPos;"
		"layout(location = 1) in vec2 inTextureCoord;"
		"layout(location = 2) in float inTextureIndex;"
		"layout(location = 3) in vec3 inNormal;"
		"layout(location = 0) out vec2 outTextureCoord;"
		"layout(location = 1) flat out float outTextureIndex;"
		"layout(location = 2) out vec3 outNormal;"
		"layout(location = 3) out vec3 outFragPos;"
		"void main()"
		"{"
		"	gl_Position = pv * vec4( inPos, 1.0f );"
		"	outFragPos = inPos;"
		"	outTextureCoord = inTextureCoord;"
		"	outTextureIndex = inTextureIndex;"
		"	outNormal = inNormal;"
		"}";

	//fragment shader
	constexpr const char* fragment_shader_code =
		"#version 430 core\n"
		"layout(binding = 0) uniform sampler2DArray textureArray;"
		"layout(location = 1) uniform vec3 viewPos;"
		"layout(location = 0) in vec2 inTextureCoord;"
		"layout(location = 1) flat in float inTextureIndex;"
		"layout(location = 2) in vec3 inNormal;"
		"layout(location = 3) in vec3 inFragPos;"
		"layout(location = 0) out vec4 outColor;"
		"void main()"
		"{"
		"	vec3 ambient = vec3(0.4f) * texture(textureArray, vec3(inTextureCoord, inTextureIndex)).rgb;"
		"	vec3 norm = normalize(inNormal);"
		"	vec3 lightDir = normalize(vec3(0.0f, 5.0f, 0.0f) - inFragPos);"
		"	float diff = max(dot(norm, lightDir), 0.0);"
		"	vec3 diffuse = vec3(0.5f) * diff * texture(textureArray, vec3(inTextureCoord, inTextureIndex)).rgb;"
		"	vec3 viewDir = normalize(viewPos - inFragPos);"
		"	vec3 reflectDir = reflect(-lightDir, norm);"
		"	float spec =  pow(max(dot(viewDir, reflectDir), 0.0), 128.0f);"
		"	vec3 specular = spec * vec3(0.1f);"
		"	outColor = vec4(ambient + diffuse + specular, 1.0);"
		"}";

	main_shader = RasterShaderProgram
	{
		vertex_shader_code,
		fragment_shader_code
	};
}

void Renderer::init_game_objects()
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<std::string> texture_strings;

	//load map from file
	{
		{
			std::ifstream map_file{ "map.sec" };

			std::vector<glm::vec2> vertices;

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
					else if (prefix_identifier.compare("player") == 0)
					{
						glm::vec2 pos;
						line_stream >> pos.x >> pos.y;

						int64_t sector = -1;
						for (size_t ii = 0; ii < sectors.size(); ii++)
						{
							for (size_t i = 0, j = sectors[ii].vertices.size() - 1; i < sectors[ii].vertices.size(); j = i++)
							{
								const auto& vert2 = sectors[ii].vertices[i];
								const auto& vert1 = sectors[ii].vertices[j];

								if ((vert1.y > pos.y) != (vert2.y > pos.y) &&
									(pos.x < (vert2.x - vert1.x) * (pos.y - vert1.y) / (vert2.y - vert1.y) + vert1.x))
								{
									sector = ii;
									break;
								}
							}
						}

						if (sector < 0)
						{
							sector = 0;
						}

						player = Player{ static_cast<uint32_t>(sector), glm::vec3{ pos.x, sectors[static_cast<size_t>(sector)].floor + Player::get_eye_height(), pos.y } };
					}
					else if (prefix_identifier.compare("sector") == 0)
					{
						Sector sector;

						//get height
						line_stream >> sector.floor >> sector.ceil;

						//get material types
						{
							uint32_t wall_type, ceil_type, floor_type;
							line_stream >> wall_type >> ceil_type >> floor_type;

							sector.wall_type = wall_type;
							sector.ceil_type = ceil_type;
							sector.floor_type = floor_type;
						}

						std::vector<int64_t> integers;
						{
							int64_t get_integer;
							while (line_stream >> get_integer)
							{
								integers.push_back(get_integer);
							}
						}

						if (integers.size() < 6)
						{
							throw std::logic_error("Sector size must have at least 3 vertices & neighbors");
						}

						const size_t size = integers.size() / 2;

						for (size_t i = 0; i < size; i++)
						{
							sector.vertices.push_back(vertices[static_cast<size_t>(integers[i])]);
							sector.neighbors.push_back(static_cast<int32_t>(integers[i + size]));
						}

						sectors.push_back(sector);
					}
					else if (prefix_identifier.compare("texture") == 0)
					{
						std::string texture_name;
						line_stream >> std::quoted(texture_name);

						texture_strings.push_back(std::move(texture_name));
					}
				}
			}
			map_file.close();
		}

		std::unordered_map<Vertex, uint32_t> unique_vertices;

		//lambda to add to indices and vertices easier
		auto add_vertex = [&vertices, &indices, &unique_vertices](const Vertex& vertex)
		{
			if (unique_vertices.count(vertex) == 0)
			{
				unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(unique_vertices[vertex]);
		};

		//change 2d sectors into 3d data
		for (auto& sector : sectors)
		{
			//create floor and ceiling
			//we use triangle fans because convex sector
			const Vertex main_floor_vert{ glm::vec3{sector.vertices[0].x, sector.floor, sector.vertices[0].y}, sector.vertices[0] / 8.0f, static_cast<float>(sector.floor_type), glm::vec3{sector.vertices[0].x, 1.0f, sector.vertices[0].y} };
			const Vertex main_ceil_vert{ glm::vec3{sector.vertices[0].x, sector.ceil, sector.vertices[0].y}, sector.vertices[0] / 8.0f, static_cast<float>(sector.ceil_type), glm::vec3{sector.vertices[0].x, -1.0f, sector.vertices[0].y} };

			for (size_t i = 1; i < (sector.vertices.size() - 1); i++)
			{
				//construct floor triangle
				{
					const Vertex floor_vert1{ glm::vec3{sector.vertices[i].x, sector.floor, sector.vertices[i].y}, sector.vertices[i] / 8.0f, static_cast<float>(sector.floor_type), glm::vec3{sector.vertices[i].x, 1.0f, sector.vertices[i].y} };
					const Vertex floor_vert2{ glm::vec3{sector.vertices[i + 1].x, sector.floor, sector.vertices[i + 1].y}, sector.vertices[i + 1] / 8.0f, static_cast<float>(sector.floor_type), glm::vec3{sector.vertices[i + 1].x, 1.0f, sector.vertices[i + 1].y} };

					add_vertex(main_floor_vert);
					add_vertex(floor_vert1);
					add_vertex(floor_vert2);
				}
				//construct ceil triangle
				{
					const Vertex ceil_vert1{ glm::vec3{sector.vertices[i].x, sector.ceil, sector.vertices[i].y}, sector.vertices[i] / 8.0f, static_cast<float>(sector.ceil_type), glm::vec3{sector.vertices[i].x, -1.0f, sector.vertices[i].y} };
					const Vertex ceil_vert2{ glm::vec3{sector.vertices[i + 1].x, sector.ceil, sector.vertices[i + 1].y}, sector.vertices[i + 1] / 8.0f, static_cast<float>(sector.ceil_type), glm::vec3{sector.vertices[i + 1].x, -1.0f, sector.vertices[i + 1].y} };

					add_vertex(ceil_vert2);
					add_vertex(ceil_vert1);
					add_vertex(main_ceil_vert);
				}
			}

			//we construct wall vertices
			for (size_t i = 0; i < sector.vertices.size(); i++)
			{
				const glm::vec2& v1 = sector.vertices[i];
				//if this is the last vertex in the list, we use the first vertex in the list as the connector
				const glm::vec2* v2;
				if (i == sector.vertices.size() - 1)
				{
					v2 = &sector.vertices[0];
				}
				else
				{
					v2 = &sector.vertices[i + 1];
				}

				const glm::vec3 normal{ (v2->y - v1.y), 0.0f, -(v2->x - v1.x) };

				const Vertex top_left{ glm::vec3{ v1.x, sector.ceil, v1.y }, glm::vec2{ ((v1.x + v1.y) * (v1.x - v1.y)) / 64.0f, sector.ceil } / 8.0f, static_cast<float>(sector.wall_type), normal };
				const Vertex top_right{ glm::vec3{ v2->x, sector.ceil, v2->y }, glm::vec2{ ((v2->x + v2->y) * (v2->x - v2->y)) / 64.0f, sector.ceil } / 8.0f, static_cast<float>(sector.wall_type), normal };
				const Vertex bottom_left{ glm::vec3{ v1.x, sector.floor, v1.y }, glm::vec2{ ((v1.x + v1.y) * (v1.x - v1.y)) / 64.0f, sector.floor } / 8.0f, static_cast<float>(sector.wall_type), normal };
				const Vertex bottom_right{ glm::vec3{ v2->x, sector.floor, v2->y }, glm::vec2{((v2->x + v2->y) * (v2->x - v2->y)) / 64.0f, sector.floor } / 8.0f, static_cast<float>(sector.wall_type), normal };

				if (sector.neighbors[i] < 0)
				{
					//no neighbor, draw solid wall
					add_vertex(top_left);
					add_vertex(top_right);
					add_vertex(bottom_left);

					add_vertex(top_right);
					add_vertex(bottom_right);
					add_vertex(bottom_left);
				}
				else
				{
					//neighbor
					const auto& neighbor_sector = sectors[sector.neighbors[i]];

					if (neighbor_sector.ceil < sector.ceil)
					{
						//math from https://math.stackexchange.com/questions/1205733/how-to-convert-or-transform-from-one-range-to-another that I probably should've already learn't and not spend 3 hours on
						//this is to make texture coords accurate and make it look like it got cut off, to fit with the other walls that are not portals
						const float normal_diff =
							((neighbor_sector.ceil - sector.floor) * (sector.ceil - sector.floor)
							/
							(sector.ceil - sector.floor)) + sector.floor;

						const Vertex neighbor_top_left{ glm::vec3{ v1.x, neighbor_sector.ceil, v1.y }, glm::vec2{ ((v1.x + v1.y) * (v1.x - v1.y)) / 64.0f, normal_diff } / 8.0f, static_cast<float>(sector.wall_type), normal };
						const Vertex neighbor_top_right{ glm::vec3{ v2->x, neighbor_sector.ceil, v2->y }, glm::vec2{ ((v2->x + v2->y) * (v2->x - v2->y)) / 64.0f,  normal_diff } / 8.0f, static_cast<float>(sector.wall_type), normal };

						add_vertex(top_left);
						add_vertex(top_right);
						add_vertex(neighbor_top_left);

						add_vertex(top_right);
						add_vertex(neighbor_top_right);
						add_vertex(neighbor_top_left);
					}

					if (neighbor_sector.floor > sector.floor)
					{
						//more math from stackexchange
						//texture coord accuracy
						const float normal_diff =
							((neighbor_sector.floor - sector.floor) * (sector.ceil - sector.floor)
								/
								(sector.ceil - sector.floor)) + sector.floor;

						const Vertex neighbor_bottom_left{ glm::vec3{ v1.x, neighbor_sector.floor, v1.y }, glm::vec2{ ((v1.x + v1.y) * (v1.x - v1.y)) / 64.0f, normal_diff } / 8.0f, static_cast<float>(sector.wall_type), normal };
						const Vertex neighbor_bottom_right{ glm::vec3{ v2->x, neighbor_sector.floor, v2->y }, glm::vec2{((v2->x + v2->y) * (v2->x - v2->y)) / 64.0f, normal_diff } / 8.0f, static_cast<float>(sector.wall_type), normal };

						add_vertex(neighbor_bottom_left);
						add_vertex(neighbor_bottom_right);
						add_vertex(bottom_left);

						add_vertex(neighbor_bottom_right);
						add_vertex(bottom_right);
						add_vertex(bottom_left);
					}
				}
			}
		}
	}

	map_mesh = Mesh{ vertices, indices };

	//copy pointers
	std::vector<const char*> textures;
	for (auto& texture_str : texture_strings)
	{
		textures.push_back(texture_str.c_str());
	}

	//build the texture array used in shaders
	texture_array = TextureArray2d{ textures, 512, 512 };
}

void Renderer::destroy_window_renderer()
{
	SDL_GL_DeleteContext(context);

	SDL_DestroyWindow(window);

	SDL_Quit();
}