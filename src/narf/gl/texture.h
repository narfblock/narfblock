#ifndef NARF_GL_TEXTURE_H
#define NARF_GL_TEXTURE_H

#include "narf/gl/gl.h"

namespace narf {
namespace gl {

class Context; // forward decl

class Texture {
public:

	Texture(Context *context) : width_(0), height_(0)
	{
		glGenTextures(1, &texture_);
	}

	~Texture()
	{
		glDeleteTextures(1, &texture_);
	}

	bool upload(SDL_Surface *surf);

	GLuint name() const { return texture_; }

	uint32_t width() const { return width_; }
	uint32_t height() const { return height_; }

private:
	GLuint texture_;

	uint32_t width_;
	uint32_t height_;
};

enum TextureTarget {
	TEXTURE_1D = GL_TEXTURE_1D,
	TEXTURE_2D = GL_TEXTURE_2D,
	TEXTURE_3D = GL_TEXTURE_3D,
	TEXTURE_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
};

} // namespace gl
} // namespace narf

// type-checking wrappers

static inline void glBindTexture(narf::gl::TextureTarget target, narf::gl::Texture *texture)
{
	glBindTexture(target, texture->name());
}

#endif // NARF_GL_TEXTURE_H