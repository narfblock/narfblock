#ifndef NARF_GL_GL_H
#define NARF_GL_GL_H

#include <stdint.h>

#include <SDL.h>
#include <SDL_opengl.h>

#ifdef __APPLE__
#   ifdef GL_ES_VERSION_2_0
#       include <OpenGLES/ES2/gl.h>
#   else
#       include <OpenGL/gl.h>
#   endif
#else
#   include <GL/gl.h>
#endif

#include "narf/gl/texture.h"
#include "narf/gl/context.h"
#include "narf/gl/buffer.h"
#include "narf/gl/shader.h"

#endif // NARF_GL_GL_H
