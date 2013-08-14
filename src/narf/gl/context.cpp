#include "narf/gl/context.h"

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
		fprintf(stderr, "ERROR: setDisplayMode: SDL_CreateWindow failed: %s\n", SDL_GetError());
		return false;
	}

	context_ = SDL_GL_CreateContext(window_);
	if (!context_) {
		fprintf(stderr, "ERROR: setDisplayMode: SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		return false;
	}

	// disable vsync
	// TODO: make this a configuration option
	if (SDL_GL_SetSwapInterval(0)) {
		fprintf(stderr, "WARNING: setDisplayMode: SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError());
	}

	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(glew_err));
		return false;
	}

	glViewport(0, 0, width, height);

	return true;
}
