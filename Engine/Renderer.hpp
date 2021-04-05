#ifndef RENDERER_OPENGL_VEOT_HPP
#define RENDERER_OPENGL_VEOT_HPP

#include <vector>
#include <array>
#include <chrono>

#include <glad/glad.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include "RasterShaderProgram.hpp"

#include "Player.hpp"

#include "RenderData.hpp"

#include "Sector.hpp"

class Renderer
{
	SDL_Window* window;
	SDL_GLContext context;

	RasterShaderProgram main_shader;

	TextureArray2d texture_array;

	Mesh map_mesh;

	std::vector<Sector> sectors;

	Player player;

	std::array<bool, 4> wasd;

	//for deltatime
	Uint64 prev_time = 0;
	double delta_time = 0;

	bool is_running;

	//window size
	int32_t window_width, window_height;

	void init_window_renderer();

	void set_sdl_settings();

	void set_opengl_settings();

	void init_game_objects();

	void destroy_window_renderer();

public:
	explicit Renderer();

	~Renderer();

	void run();

private:
	void get_events();

	void handle_events();

	void draw();
};

#endif