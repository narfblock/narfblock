/*
 * NarfBlock client world class
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

#ifndef NARF_CLIENT_WORLD_H
#define NARF_CLIENT_WORLD_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // TODO: for log; remove later

#include "narf/world.h"

#include "narf/client/chunk.h"
#include "narf/camera.h"
#include "narf/vector.h"
#include "narf/math/math.h"

#include "narf/gl/gl.h"

namespace narf {
namespace client {

class World : public narf::World {
public:

	World(uint32_t size_x, uint32_t size_y, uint32_t size_z, uint32_t chunk_size_x, uint32_t chunk_size_y, uint32_t chunk_size_z) :
		narf::World(size_x, size_y, size_z, chunk_size_x, chunk_size_y, chunk_size_z)
	{
	}

	void render(narf::gl::Texture *tiles_tex, const narf::Camera *cam);

protected:
	Chunk *new_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z) {
		return new narf::client::Chunk(this,
		                 chunk_size_x_, chunk_size_y_, chunk_size_z_,
		                 chunk_x * chunk_size_x_, chunk_y * chunk_size_y_, chunk_z * chunk_size_z_);
	}

	Chunk *get_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z) {
		return static_cast<narf::client::Chunk*>(narf::World::get_chunk(chunk_x, chunk_y, chunk_z));
	}
};

} // namespace client
} // namespace narf

#endif // NARF_CLIENT_WORLD_H
