#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>
#include <algorithm>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.hpp"

#include "Sector.hpp"

//the programming in here might be a bit shoddy, due to this being a one-off

//oh god the static variables in here

static GLFWwindow* window = nullptr;

#ifndef NDEBUG
static int width = 1124, height = 894;
#else
static int width = 1600, height = 900;
#endif

static GLuint shader_program = 0;

static bool is_z_pressed = false;
static bool is_p_pressed = false;
static bool is_g_pressed = false;
static bool is_n_pressed = false;

static bool camera_locked = false;
static cam::Camera camera{ glm::vec3{ 1.0f } };

static bool is_sprinting = false;

static bool is_making_sector = false;

static bool draw_grid = true;

static std::vector<Sector> sectors;

struct Renderable
{
	GLuint vao, vbo;
	GLsizei size;
};

static Renderable cube{};
static glm::vec3 cube_pos{ 0.0f };

static std::vector<glm::vec3> cube_vert_poses{};

static glm::vec3 org_cube_vert_pos{ 0.0f };

static Renderable grid{};

static std::vector<Renderable> sector_meshes;
static std::vector<Renderable> sector_wireframe_meshes;

void create_shader_program()
{
	constexpr auto vertex_shader_code =
		"#version 430 core\n"
		"layout (location = 0) uniform mat4 proj_view_mat;"
		"layout (location = 1) uniform mat4 model_mat;"
		"layout (location = 0) in vec3 pos;"
		"void main()"
		"{"
		"	gl_Position = proj_view_mat * model_mat * vec4(pos, 1.0);"
		"}";

	constexpr auto fragment_shader_code =
		"#version 430 core\n"
		"layout (location = 2) uniform vec3 vert_colour;"
		"layout (location = 0) out vec4 out_colour;"
		"void main()"
		"{"
		"	out_colour = vec4(vert_colour, 1.0f);"
		"}";

	auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_code, nullptr);
	glCompileShader(vertex_shader);

	auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_code, nullptr);
	glCompileShader(fragment_shader);

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

void create_cube()
{
	glGenVertexArrays(1, &cube.vao);
	glGenBuffers(1, &cube.vbo);

	constexpr std::array<float, 108> vertices =
	{
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f
	};

	glBindVertexArray(cube.vao);

	glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	cube.size = static_cast<GLsizei>(vertices.size());
}

void create_grid()
{
	glGenVertexArrays(1, &grid.vao);
	glGenBuffers(1, &grid.vbo);

	//build grid data
	std::vector<float> vertices;
	for (int i = 0; i < 501; i++)
	{
		vertices.push_back(-250.0f);
		vertices.push_back(0.0f);
		vertices.push_back(static_cast<float>(i) - 250.0f);

		vertices.push_back(250.0f);
		vertices.push_back(0.0f);
		vertices.push_back(static_cast<float>(i) - 250.0f);
	}

	for (int i = 0; i < 501; i++)
	{
		vertices.push_back(static_cast<float>(i) - 250.0f);
		vertices.push_back(0.0f);
		vertices.push_back(-250.0f);

		vertices.push_back(static_cast<float>(i) - 250.0f);
		vertices.push_back(0.0f);
		vertices.push_back(250.0f);
	}

	glBindVertexArray(grid.vao);

	glBindBuffer(GL_ARRAY_BUFFER, grid.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	grid.size = static_cast<GLsizei>(vertices.size());
}

bool is_point_in_sector(glm::vec2 point, const Sector& sector)
{
	bool result = false;
	for (size_t i = 0, j = sector.vertices.size() - 1; i < sector.vertices.size(); j = i++)
	{
		const auto& vert1 = sector.vertices[i];
		const auto& vert2 = sector.vertices[j];

		if ((vert1.y > point.y) != (vert2.y > point.y) &&
			(point.x < (vert2.x - vert1.x) * (point.y - vert1.y) / (vert2.y - vert1.y) + vert1.x))
		{
			result = !result;
		}
	}

	return result;
}

Sector* sector_cube_is_in()
{
	if (sectors.empty())
	{
		return nullptr;
	}

	for (auto& sector : sectors)
	{
		if (is_point_in_sector(cube_pos, sector))
		{
			return &sector;
		}
	}

	return nullptr;
}

//keyboard press input
void process_input();

//framebuffer callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	::width = width;
	::height = height;
	glViewport(0, 0, width, height);
}

void write_sectors_to_file()
{
	std::ofstream file{ "map.sec" };

	file << "texture \"wall.jpg\"\n" << "texture \"container.jpg\"\n" << "texture \"stone.jpg\"\n";

	uint32_t offset = 0;
	for (const auto& sector : sectors)
	{
		uint32_t temp_offset = 0;

		for (const auto& vertex : sector.vertices)
		{
			file << "vertex " << vertex.x << ' ' << vertex.y << '\n';
		}

		temp_offset = static_cast<uint32_t>(sector.vertices.size());

		file << "sector " << sector.floor << ' ' << sector.ceil << ' ' << sector.wall_type << ' ' << sector.ceil_type << ' ' << sector.floor_type << ' ';

		for (size_t i = 0; i < sector.vertices.size(); i++)
		{
			file << i + offset << ' ';
		}

		for (const auto& neighbor : sector.neighbors)
		{
			file << neighbor << ' ';
		}

		file << '\n';

		offset += temp_offset;
	}

	file.close();
}

void make_neighbors_for_sectors()
{
	for (auto& sector : sectors)
	{
		for (size_t i = 0; i < sector.vertices.size(); i++)
		{
			const auto& vertex = sector.vertices[i];
			for (size_t s = 0; s < sectors.size(); s++)
			{
				const auto& other_sector = sectors[s];
				if (&other_sector == &sector)
				{
					continue;
				}

				for (size_t ii = 0; ii < other_sector.vertices.size(); ii++)
				{
					const auto& other_vertex = other_sector.vertices[ii];

					const glm::vec2* vertex2 = nullptr;
					if (i == sector.vertices.size() - 1)
					{
						vertex2 = &sector.vertices[0];
					}
					else
					{
						vertex2 = &sector.vertices[i + 1];
					}

					const glm::vec2* other_vertex2 = nullptr;
					if (ii == other_sector.vertices.size() - 1)
					{
						other_vertex2 = &other_sector.vertices[0];
					}
					else
					{
						other_vertex2 = &other_sector.vertices[ii + 1];
					}

					if (*vertex2 == other_vertex)
					{
						if (*other_vertex2 == vertex)
						{
							sector.neighbors[i] = static_cast<int32_t>(s);
						}
					}
				}
			}
		}
	}
}

int main(int argc, char** argv)
{
	glfwInit();
	//could probably use 3.3 but 4.3 has convinient setting of uniform locations
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Map Editor", nullptr, nullptr);
	if (window == nullptr)
	{
		//crash
		throw std::runtime_error("Failed to create window");
	}
	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//set vsync
	glfwSwapInterval(0);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		//crash
		throw std::runtime_error("Failed to load func pointers");
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_LINE_SMOOTH);

	glViewport(0, 0, width, height);

	glClearColor(0.0f, 0.4f, 0.4f, 1.0f);

	create_shader_program();

	create_cube();

	create_grid();

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader_program);

		//set perspective view
		const auto view = camera.GetViewMatrix();

		const auto perspective = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 700.0f);

		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(perspective * view));

		//draw grid
		if (draw_grid)
		{
			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

			glUniform3f(2, 0.2f, 0.2f, 0.2f);

			glBindVertexArray(grid.vao);

			glDrawArrays(GL_LINES, 0, grid.size);
		}

		//draw 0,0 point
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3{ 0.0f, 10.0f, 0.0f }), glm::vec3(0.1f, 20.0f, 0.1f))));

		glUniform3f(2, 0.0f, 1.0f, 0.0f);

		glBindVertexArray(cube.vao);

		glDrawArrays(GL_TRIANGLES, 0, cube.size);

		//draw x bar
		{
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3{ 0.0f, 10.0f, 0.0f });
			transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
			transform = glm::scale(transform, glm::vec3(0.1f, 20.0f, 0.1f));
			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(transform));

			glUniform3f(2, 1.0f, 0.0f, 0.0f);

			glBindVertexArray(cube.vao);

			glDrawArrays(GL_TRIANGLES, 0, cube.size);
		}

		//draw y bar
		{
			glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3{ 0.0f, 10.0f, 0.0f });
			transform = glm::rotate(transform, glm::radians(90.0f), glm::vec3{ 0.0f, 0.0f, 1.0f });
			transform = glm::scale(transform, glm::vec3(0.1f, 20.0f, 0.1f));
			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(transform));

			glUniform3f(2, 0.0f, 0.0f, 1.0f);

			glBindVertexArray(cube.vao);

			glDrawArrays(GL_TRIANGLES, 0, cube.size);
		}

		//draw cube
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::translate(glm::mat4(1.0f), cube_pos), glm::vec3(0.4f))));

		glUniform3f(2, 1.0f, 1.0f, 1.0f);

		glBindVertexArray(cube.vao);

		glDrawArrays(GL_TRIANGLES, 0, cube.size);

		//draw temporary vert cube
		if (is_making_sector)
		{
			for (const auto& cube_vert_pos : cube_vert_poses)
			{
				glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::translate(glm::mat4(1.0f), cube_vert_pos), glm::vec3(0.3f))));

				glUniform3f(2, 1.0f, 0.0f, 1.0f);

				glBindVertexArray(cube.vao);

				glDrawArrays(GL_TRIANGLES, 0, cube.size);
			}

			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::translate(glm::mat4(1.0f), org_cube_vert_pos), glm::vec3(0.4f))));

			glUniform3f(2, 0.0f, 1.0f, 1.0f);

			glBindVertexArray(cube.vao);

			glDrawArrays(GL_TRIANGLES, 0, cube.size);
		}

		//draw blue plain
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

		glUniform3f(2, 0.0f, 0.0f, 1.0f);
		for (const auto& sector_mesh : sector_meshes)
		{
			glBindVertexArray(sector_mesh.vao);
			
			glDrawArrays(GL_TRIANGLES, 0, sector_mesh.size);
		}

		//draw wireframe
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

		glUniform3f(2, 0.6f, 0.0f, 1.0f);
		for (const auto& sector_mesh : sector_wireframe_meshes)
		{
			glBindVertexArray(sector_mesh.vao);

			glDrawArrays(GL_LINES, 0, sector_mesh.size);
		}

		//draw verts on each sector vert
		//generate on the fly matrix locations
		glUniform3f(2, 1.0f, 1.0f, 0.0f);
		glBindVertexArray(cube.vao);
		for (const auto& sector : sectors)
		{
			for (const auto& vert : sector.vertices)
			{
				glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3{ vert.x, 0.0f, vert.y }), glm::vec3(0.1f))));

				glDrawArrays(GL_TRIANGLES, 0, cube.size);
			}
		}

		glfwSwapBuffers(window);

		//call before glfwPollEvents to get the previous frame's cursor pos
		static double prev_xpos = 0.0, prev_ypos = 0.0;
		glfwGetCursorPos(window, &prev_xpos, &prev_ypos);

		glfwPollEvents();

		process_input();

		//mouse movement
		if (camera_locked)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			xpos = xpos - prev_xpos;
			ypos = prev_ypos - ypos;

			camera.ProcessMouseMovement(static_cast<float>(xpos), static_cast<float>(ypos));
		}
	}

	glfwTerminate();

	return 0;
}

void process_input()
{
	static float last_frame = 0.0f;
	float current_frame = static_cast<float>(glfwGetTime());
	float deltatime = current_frame - last_frame;
	last_frame = current_frame;

	//TODO: PUT THIS COMMENT IN A TEXT FILE
	/*
	* 
	* Z locks and unlocks the camera
	* 
	* 
	* If camera is locked:
	* WASD moves the camera
	* LSHIFT moves the camera at 10x speed
	* R resets the Position, Yaw, and Pitch of the Camera to (0, 0)
	* 
	* If camera is not locked
	* WASD moves the block along the grid, and is clamped to the edges (500, 500)
	* Pressing L locks the cube to the nearest 1:1 coordinate
	* LSHIFT moves the block at 10x speed
	* R resets the block to (0, 0)
	* P plots down a vertex
	* G turns off and on the grid
	* 
	*/

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (camera_locked)
		{
			camera.ProcessKeyboard(cam::Movement::Forward, deltatime, is_sprinting);
		}
		else
		{
			cube_pos.z -= (is_sprinting ? 50.0f : 5.0f) * deltatime;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (camera_locked)
		{
			camera.ProcessKeyboard(cam::Movement::Backward, deltatime, is_sprinting);
		}
		else
		{
			cube_pos.z += (is_sprinting ? 50.0f : 5.0f) * deltatime;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (camera_locked)
		{
			camera.ProcessKeyboard(cam::Movement::Left, deltatime, is_sprinting);
		}
		else
		{
			cube_pos.x -= (is_sprinting ? 50.0f : 5.0f) * deltatime;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (camera_locked)
		{
			camera.ProcessKeyboard(cam::Movement::Right, deltatime, is_sprinting);
		}
		else
		{
			cube_pos.x += (is_sprinting ? 50.0f : 5.0f) * deltatime;
		}
	}

	cube_pos.x = std::clamp(cube_pos.x, -250.0f, 250.0f);
	cube_pos.z = std::clamp(cube_pos.z, -250.0f, 250.0f);

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		if (!is_z_pressed)
		{
			camera_locked = !camera_locked;

			if (camera_locked)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
		is_z_pressed = true;
	}
	else
	{
		is_z_pressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		is_sprinting = true;
	}
	else
	{
		is_sprinting = false;
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		if (camera_locked)
		{
			camera.Reset();
		}
		else
		{
			cube_pos = glm::vec3{ 0.0f };
		}
	}

	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
	{
		if (!camera_locked)
		{
			write_sectors_to_file();
		}
	}

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		if (!camera_locked)
		{
			static Sector sector;

			if (!is_p_pressed)
			{
				if (!is_making_sector)
				{
					//start
					is_making_sector = true;

					sector = Sector
					{
						10.0f, 0.0f,
						0, 1, 2
					};

					cube_pos.x = round(cube_pos.x);
					cube_pos.z = round(cube_pos.z);

					const glm::vec2 vert{ cube_pos.x, cube_pos.z };

					const glm::vec3 vert3d{ vert.x, 0.0f, vert.y };

					cube_vert_poses.clear();

					org_cube_vert_pos = vert3d;

					sector.vertices.push_back(vert);

					sector.neighbors.push_back(-1);
				}
				else
				{
					//continue
					cube_pos.x = round(cube_pos.x);
					cube_pos.z = round(cube_pos.z);

					const glm::vec2 vert{ cube_pos.x, cube_pos.z };

					if (vert == sector.vertices[0])
					{
						sectors.push_back(sector);

						//turn into mesh to draw
						{
							Renderable sector_mesh;

							glGenVertexArrays(1, &sector_mesh.vao);
							glGenBuffers(1, &sector_mesh.vbo);

							glBindVertexArray(sector_mesh.vao);

							std::vector<glm::vec3> vertices;

							const glm::vec3 main_vert{ sector.vertices[0].x, 0.0f, sector.vertices[0].y };
							for (size_t i = 1; i < sector.vertices.size() - 1; i++)
							{
								const glm::vec3 vert1{ sector.vertices[i].x, 0.0f, sector.vertices[i].y };
								const glm::vec3 vert2{ sector.vertices[i + 1].x, 0.0f, sector.vertices[i + 1].y };

								vertices.push_back(main_vert);
								vertices.push_back(vert1);
								vertices.push_back(vert2);
							}

							glBindBuffer(GL_ARRAY_BUFFER, sector_mesh.vbo);
							glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

							glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
							glEnableVertexAttribArray(0);

							sector_mesh.size = static_cast<GLsizei>(vertices.size());

							sector_meshes.push_back(std::move(sector_mesh));
						}
						//make outline
						{
							Renderable sector_wireframe_mesh;

							glGenVertexArrays(1, &sector_wireframe_mesh.vao);
							glGenBuffers(1, &sector_wireframe_mesh.vbo);

							glBindVertexArray(sector_wireframe_mesh.vao);

							std::vector<glm::vec3> vertices;
							for (size_t i = 0; i < sector.vertices.size(); i++)
							{
								vertices.push_back(glm::vec3(sector.vertices[i].x, 0.0f, sector.vertices[i].y));

								if (i == sector.vertices.size() - 1)
								{
									vertices.push_back(glm::vec3(sector.vertices[0].x, 0.0f, sector.vertices[0].y));
								}
								else
								{
									vertices.push_back(glm::vec3(sector.vertices[i + 1].x, 0.0f, sector.vertices[i + 1].y));
								}
							}

							glBindBuffer(GL_ARRAY_BUFFER, sector_wireframe_mesh.vbo);
							glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

							glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
							glEnableVertexAttribArray(0);

							sector_wireframe_mesh.size = static_cast<GLsizei>(vertices.size());

							sector_wireframe_meshes.push_back(std::move(sector_wireframe_mesh));
						}

						make_neighbors_for_sectors();

						is_making_sector = false;
					}
					else
					{
						glm::vec3 cube_vert_pos{ vert.x, 0.0f, vert.y };

						cube_vert_poses.push_back(std::move(cube_vert_pos));

						sector.vertices.push_back(vert);

						sector.neighbors.push_back(-1);
					}
				}
			}

			is_p_pressed = true;
		}
	}
	else
	{
		if (!camera_locked)
		{
			is_p_pressed = false;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		if (!camera_locked)
		{
			cube_pos.x = round(cube_pos.x);
			cube_pos.z = round(cube_pos.z);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		if (!camera_locked)
		{
			if (!is_g_pressed)
			{
				draw_grid = !draw_grid;
			}
		}

		is_g_pressed = true;
	}
	else
	{
		if (!camera_locked)
		{
			is_g_pressed = false;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		if (!is_n_pressed)
		{
			if (!sectors.empty())
			{
				sectors.pop_back();
				sector_meshes.pop_back();
				sector_wireframe_meshes.pop_back();
			}
		}

		is_n_pressed = true;
	}
	else
	{
		is_n_pressed = false;
	}
}