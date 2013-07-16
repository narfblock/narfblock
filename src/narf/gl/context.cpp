#include "narf/gl/context.h"

narf::gl::Context::Context()
{
	load_extensions();
}


bool narf::gl::Context::set_display_mode(int width, int height, int bpp, bool fullscreen)
{
	Uint32 flags = SDL_HWSURFACE | SDL_OPENGL;
	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// disable vsync
	// TODO: make this a configuration option
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);


	surface_ = SDL_SetVideoMode(width, height, bpp, flags);
	if (!surface_) {
		return false;
	}

	glViewport(0, 0, width, height);

	return true;
}
