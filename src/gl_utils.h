#ifndef PLOTTER_GL_UTILS_H
#define PLOTTER_GL_UTILS_H

#ifdef WIN32
#include <SDL.h>
#undef main
#else
#include <SDL2/SDL.h>
#endif

#include <GL/glew.h>

#include <memory>


std::string to_string(std::string_view str);
void sdl2_fail(std::string_view message);
void glew_fail(std::string_view message, GLenum error);

GLuint create_shader(GLenum type, const char * source);
GLuint create_program(GLuint vertex_shader, GLuint fragment_shader);

SDL_Window* sdl_init_window(std::string title);



#endif
