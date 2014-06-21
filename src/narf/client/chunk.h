/*
 * NarfBlock client chunk class
 *
 * Copyright (c) 2013 Daniel Verkamp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NARF_CLIENT_CHUNK_H
#define NARF_CLIENT_CHUNK_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include "narf/world.h"

#include "narf/block.h"
#include "narf/chunk.h"
#include "narf/math/vector.h"
#include "narf/gl/gl.h"

namespace narf {
namespace client {

class World;

struct BlockVertex {
	GLfloat vertex[3];
	GLfloat color[3];
	GLfloat texcoord[2];
};

class Chunk : public narf::Chunk {
public:

	Chunk(
		narf::World *world,
		uint32_t size_x, uint32_t size_y, uint32_t size_z,
		uint32_t pos_x, uint32_t pos_y, uint32_t pos_z) : narf::Chunk(world, size_x, size_y, size_z, pos_x, pos_y, pos_z),
		vbo_(GL_ARRAY_BUFFER, GL_STATIC_DRAW)
	{
	}

	~Chunk()
	{
	}

	void render();

private:
	narf::gl::Buffer<BlockVertex> vbo_;

	// internal rendering functions
	void build_vertex_buffers();
	void draw_quad(narf::gl::Buffer<BlockVertex> &vbo, const BlockTexCoord &texCoord, const float *quad);
};

} // namespace client
} // namespace narf

#endif // NARF_CLIENT_CHUNK_H
