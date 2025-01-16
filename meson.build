project('SectorRenderer', 'c', 'cpp')

sdl2_dep = dependency('sdl2')
glfw3_dep = dependency('glfw3')
glm_dep = dependency('glm')

glad_inc = include_directories('glad/include')
stb_inc = include_directories('stb/include')

executable('Engine',
	'Engine/Camera.cpp', 'Engine/Player.cpp', 'Engine/RasterShaderProgram.cpp',
	'Engine/RenderData.cpp', 'Engine/Renderer.cpp', 'Engine/main.cpp',
	'glad/src/glad.c', 'stb/src/stb_image.cpp',
	include_directories : [glad_inc, stb_inc],
	dependencies : [sdl2_dep, glm_dep])

executable('MapEditor',
	'MapEditor/main.cpp',
	'MapEditor/Camera.cpp',
	'glad/src/glad.c', 'stb/src/stb_image.cpp',
	include_directories : [glad_inc, stb_inc],
	dependencies : [glfw3_dep, glm_dep])
