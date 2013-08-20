#include "narf/gl/context.h"
#include "narf/console.h"

narf::gl::Context::Context()
{
}


bool narf::gl::Context::setDisplayMode(const char *title, int width, int height, bool fullscreen)
{
	Uint32 flags = SDL_WINDOW_OPENGL;
	if (fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (!window_) {
		console->println("ERROR: setDisplayMode: SDL_CreateWindow failed: " + std::string(SDL_GetError()));
		return false;
	}

	context_ = SDL_GL_CreateContext(window_);
	if (!context_) {
		console->println("ERROR: setDisplayMode: SDL_GL_CreateContext failed: " + std::string(SDL_GetError()));
		return false;
	}

	// disable vsync
	// TODO: make this a configuration option
	if (SDL_GL_SetSwapInterval(0)) {
		console->println("WARNING: setDisplayMode: SDL_GL_SetSwapInterval failed: " + std::string(SDL_GetError()));
	}

	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		console->println("Error initializing GLEW: " + std::string((const char *)glewGetErrorString(glew_err)));
		return false;
	}

	glViewport(0, 0, width, height);

	return true;
}
