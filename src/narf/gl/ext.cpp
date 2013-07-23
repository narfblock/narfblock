#include <assert.h>

#include "narf/gl/gl.h"

PFNGLGENBUFFERSARBPROC glGenBuffers;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;
PFNGLBINDBUFFERARBPROC glBindBuffer;
PFNGLBUFFERDATAARBPROC glBufferData;

void narf::gl::load_extensions()
{
	// TODO: check if extensions are present
	glGenBuffers = (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
	glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");
	glBindBuffer = (PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
	glBufferData = (PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
}
