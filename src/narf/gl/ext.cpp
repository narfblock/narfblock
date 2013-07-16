#include "narf/gl/gl.h"

PFNGLGENBUFFERSARBPROC glGenBuffers;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;

void narf::gl::load_extensions()
{
	// TODO: check if extensions are present
	glGenBuffers = (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
	glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");
}
