/*
 * NarfBlock world class
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

#ifndef NARF_WORLD_H
#define NARF_WORLD_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // TODO: for log; remove later

#include <vector>

#include "narf/chunk.h"
#include "narf/entity.h"
#include "narf/math/math.h"

namespace narf {

// TODO: move this to util or something
static uint32_t ilog2(uint32_t v)
{
	// TODO: do a better implementation
	return (uint32_t)(log((double)v) / log(2.0));
}

class World {
public:

	World(uint32_t size_x, uint32_t size_y, uint32_t size_z, uint32_t chunk_size_x, uint32_t chunk_size_y, uint32_t chunk_size_z) :
		size_x_(size_x), size_y_(size_y), size_z_(size_z),
		chunk_size_x_(chunk_size_x), chunk_size_y_(chunk_size_y), chunk_size_z_(chunk_size_z)
	{
		// TODO: verify size is a multiple of chunk_size and a power of 2

		// world coordinate masks for wraparound
		// z does not wrap around
		mask_x_ = size_x_ - 1;
		mask_y_ = size_y_ - 1;

		// chunk shifts to get chunk coords from world coords
		chunk_shift_x_ = ilog2(chunk_size_x);
		chunk_shift_y_ = ilog2(chunk_size_y);
		chunk_shift_z_ = ilog2(chunk_size_z);

		// block masks to get block coords within chunk from world coords
		block_mask_x_ = (1u << chunk_shift_x_) - 1;
		block_mask_y_ = (1u << chunk_shift_y_) - 1;
		block_mask_z_ = (1u << chunk_shift_z_) - 1;

		// calculate size of world in chunks
		chunks_x_ = size_x_ / chunk_size_x;
		chunks_y_ = size_y_ / chunk_size_y;
		chunks_z_ = size_z_ / chunk_size_z;

		chunk_mask_x_ = chunks_x_ - 1;
		chunk_mask_y_ = chunks_y_ - 1;

		chunks_ = (Chunk**)calloc(chunks_x_ * chunks_y_ * chunks_z_, sizeof(Chunk*));
	}

	virtual ~World()
	{
		for (uint32_t i = 0; i < size_x_ * size_y_ * size_z_; i++) {
			if (chunks_[i]) {
				delete chunks_[i];
			}
		}
		free(chunks_);
	}

	const Block *get_block(uint32_t x, uint32_t y, uint32_t z)
	{
		uint32_t cx, cy, cz, bx, by, bz;
		calc_chunk_coords(x, y, z, &cx, &cy, &cz, &bx, &by, &bz);
		Chunk *chunk = get_chunk(cx, cy, cz);
		return chunk->get_block(bx, by, bz);
	}

	virtual void put_block(const Block *b, uint32_t x, uint32_t y, uint32_t z) {
		uint32_t cx, cy, cz, bx, by, bz;
		calc_chunk_coords(x, y, z, &cx, &cy, &cz, &bx, &by, &bz);
		Chunk *chunk = get_chunk(cx, cy, cz);
		chunk->put_block(b, bx, by, bz);
	}

	bool is_opaque(uint32_t x, uint32_t y, uint32_t z)
	{
		uint32_t cx, cy, cz, bx, by, bz;
		calc_chunk_coords(x, y, z, &cx, &cy, &cz, &bx, &by, &bz);
		Chunk *chunk = get_chunk(cx, cy, cz);
		return chunk->is_opaque(bx, by, bz);
	}

	uint32_t size_x() const { return size_x_; }
	uint32_t size_y() const { return size_y_; }
	uint32_t size_z() const { return size_z_; }

	uint32_t chunks_x() const { return chunks_x_; }
	uint32_t chunks_y() const { return chunks_y_; }
	uint32_t chunks_z() const { return chunks_z_; }

	void set_gravity(float g) { gravity_ = g; }
	float get_gravity() { return gravity_; }

	BlockWrapper rayTrace(narf::math::coord::Point3f baseLocation, narf::math::Orientationf orientation, float max_distance) {
		float distance = 0;
		auto ray = narf::math::coord::Sphericalf(distance, (float)M_PI/2 - orientation.pitch, -((float)orientation.yaw - (float)M_PI/2));
		auto rayEnd = narf::math::coord::Point3f(0, 0, 0);
		int32_t prevX = 0;
		int32_t prevY = 0;
		int32_t prevZ = 0;
		const Block* block;
		bool found = false;
		for (; distance < max_distance; distance += 0.05f) {
			ray.radius = distance;
			rayEnd = baseLocation + ray;
			block = get_block((uint32_t)rayEnd.x, (uint32_t)rayEnd.y, (uint32_t)rayEnd.z);
			if (block->id != 0) {
				found = true;
				break;
			}
			prevX = prevX != (int)rayEnd.x ? (int)rayEnd.x : prevX;
			prevY = prevY != (int)rayEnd.y ? (int)rayEnd.y : prevY;
			prevZ = prevZ != (int)rayEnd.z ? (int)rayEnd.z : prevZ;
		}

		if (!found) {
			BlockWrapper tmp = {nullptr};
			return tmp;
		}

		BlockFace face = narf::Invalid;
		int32_t xDiff = prevX - (int)rayEnd.x;
		int32_t yDiff = prevY - (int)rayEnd.y;
		int32_t zDiff = prevZ - (int)rayEnd.z;
		if (xDiff != 0) {
			if (xDiff < 0) {
				face = narf::XNeg;
			} else {
				face = narf::XPos;
			}
		} else if (yDiff != 0) {
			if (yDiff < 0) {
				face = narf::YNeg;
			} else {
				face = narf::YPos;
			}
		} else if (zDiff != 0) {
			if (zDiff < 0) {
				face = narf::ZNeg;
			} else {
				face = narf::ZPos;
			}
		}

		BlockWrapper tmp = {block, static_cast<int32_t>(rayEnd.x), static_cast<int32_t>(rayEnd.y), static_cast<int32_t>(rayEnd.z), face};
		return tmp;
	}

	Entity* newEntity() {
		auto ent = new Entity(this);
		entities_.push_back(ent);
		return ent;
	}

	void update(double t, double dt);

protected:

	Chunk **chunks_;

	uint32_t size_x_, size_y_, size_z_; // size of the world in blocks
	uint32_t mask_x_, mask_y_;

	uint32_t chunk_size_x_, chunk_size_y_, chunk_size_z_;

	uint32_t chunks_x_, chunks_y_, chunks_z_; // size of the world in chunks
	uint32_t chunk_mask_x_, chunk_mask_y_;
	uint32_t chunk_shift_x_, chunk_shift_y_, chunk_shift_z_;
	uint32_t block_mask_x_, block_mask_y_, block_mask_z_;

	float gravity_;

	std::vector<narf::Entity*> entities_;


	void calc_chunk_coords(
		uint32_t x, uint32_t y, uint32_t z,
		uint32_t *chunk_x, uint32_t *chunk_y, uint32_t *chunk_z,
		uint32_t *block_x, uint32_t *block_y, uint32_t *block_z) const
	{
		// wrap around
		x = x & mask_x_;
		y = y & mask_y_;

		// clamp z to world height
		// TODO
		if (z >= size_z_) {
			z = size_z_ - 1;
		}

		*chunk_x = x >> chunk_shift_x_;
		*chunk_y = y >> chunk_shift_y_;
		*chunk_z = z >> chunk_shift_z_;

		*block_x = x & block_mask_x_;
		*block_y = y & block_mask_y_;
		*block_z = z & block_mask_z_;
	}

	virtual Chunk *new_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z) {
		return new Chunk(
			this,
			chunk_size_x_, chunk_size_y_, chunk_size_z_,
			chunk_x * chunk_size_x_, chunk_y * chunk_size_y_, chunk_z * chunk_size_z_);
	}

	Chunk *get_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z)
	{
		chunk_x &= chunk_mask_x_;
		chunk_y &= chunk_mask_y_;
		assert(chunk_z < chunks_z_);
		Chunk *chunk = chunks_[((chunk_z * chunks_y_) + chunk_y) * chunks_x_ + chunk_x];
		if (!chunk) {
			// get from backing store, or allocate if it doesn't exist yet
			// for now, no backing store, so just always allocate a new chunk
			chunk = chunks_[((chunk_z * chunks_y_) + chunk_y) * chunks_x_ + chunk_x] =
				new_chunk(chunk_x, chunk_y, chunk_z);
			chunk->generate();
		}
		return chunk;
	}

};

} // namespace narf

#endif // NARF_WORLD_H
