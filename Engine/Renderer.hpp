#ifndef RENDERER_OPENGL_VEOT_HPP
#define RENDERER_OPENGL_VEOT_HPP

#include <vector>
#include <array>

#include <glad/glad.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include "ShaderProgram.hpp"

#include "Camera.hpp"

#include "RenderData.hpp"

class Renderer
{
	SDL_Window* window;
	SDL_GLContext context;

	ShaderProgram main_shader;

	Mesh batch_mesh;

	cam::Camera camera;

	std::array<bool, 4> wasd;

	//for deltatime
	Uint64 prev_time = 0, current_time = 0;
	float delta_time = 0.0f;

	bool is_running;

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