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
		narf::World(size_x, size_y, size_z, chunk_size_x, chunk_size_y, chunk_size_z), renderDistance(1)
	{
	}

	void render(narf::gl::Texture *tiles_tex, const narf::Camera *cam);

	void put_block(const Block *b, uint32_t x, uint32_t y, uint32_t z) {
		uint32_t cx, cy, cz, bx, by, bz;
		calc_chunk_coords(x, y, z, &cx, &cy, &cz, &bx, &by, &bz);
		Chunk *chunk = get_chunk(cx, cy, cz);
		chunk->put_block(b, bx, by, bz);
		chunk->rebuild_vertex_buffers();

		// update neighboring chunk meshes since they may have holes exposed by removing this block
		// or extra faces that are obstructed by adding this block
		if (bx == 0) get_chunk((cx - 1) & mask_x_, cy, cz)->rebuild_vertex_buffers();
		if (by == 0) get_chunk(cx, (cy - 1) & mask_y_, cz)->rebuild_vertex_buffers();
		if (bz == 0 && cz > 0) get_chunk(cx, cy, cz - 1)->rebuild_vertex_buffers();
		if (bx == chunk_size_x_ - 1) get_chunk((cx + 1) & mask_x_, cy, cz)->rebuild_vertex_buffers();
		if (by == chunk_size_y_ - 1) get_chunk(cx, (cy + 1) & mask_y_, cz)->rebuild_vertex_buffers();
		if (bz == chunk_size_z_ - 1 && cz < chunks_z_ - 1) get_chunk(cx, cy, cz + 1)->rebuild_vertex_buffers();
	}

	int32_t renderDistance; // radius in chunks

protected:
	Chunk *new_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z) {
		return new narf::client::Chunk(this,
		                 chunk_size_x_, chunk_size_y_, chunk_size_z_,
		                 chunk_x * chunk_size_x_, chunk_y * chunk_size_y_, chunk_z * chunk_size_z_);
	}

	Chunk *get_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z) {
		return static_cast<narf::client::Chunk*>(narf::World::get_chunk(chunk_x, chunk_y, chunk_z));
	}

	void renderSlice(narf::gl::Texture *tiles_tex, uint32_t cx_min, uint32_t cx_max, uint32_t cy_min, uint32_t cy_max);
};

} // namespace client
} // namespace narf

#endif // NARF_CLIENT_WORLD_H
