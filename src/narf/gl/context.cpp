#include "narf/gl/context.h"

bool narf::gl::Context::set_display_mode(int width, int height, int bpp, bool fullscreen)
{
	Uint32 flags = SDL_HWSURFACE | SDL_OPENGL;
	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	surface_ = SDL_SetVideoMode(width, height, bpp, flags);
	if (!surface_) {
		return false;
	}

	glViewport(0, 0, width, height);

	return true;
}