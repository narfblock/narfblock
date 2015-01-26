/*
 * NarfBlock chunk class
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

#ifndef NARF_CHUNK_H
#define NARF_CHUNK_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#include "narf/block.h"
#include "narf/bytestream.h"
#include "narf/math/coord3D.h"

namespace narf {

class World;

class Chunk {
public:

	// block coordinate within chunk
	typedef Point3<int32_t> BlockCoord;

	Chunk(
		World *world,
		int32_t sizeX, int32_t sizeY, int32_t sizeZ,
		int32_t pos_x, int32_t pos_y, int32_t pos_z) :
		world_(world),
		sizeX_(sizeX), sizeY_(sizeY), sizeZ_(sizeZ),
		pos_x_(pos_x), pos_y_(pos_y), pos_z_(pos_z),
		dirty_(false)
	{
		blocks_ = (Block*)calloc(static_cast<size_t>(sizeX_ * sizeY_ * sizeZ_), sizeof(Block));
	}

	virtual ~Chunk()
	{
		free(blocks_);
	}

	void serialize(ByteStreamWriter& s);
	void deserialize(ByteStreamReader& s);

	void generate(); // generate terrain for a fresh chunk

	// coordinates are relative to chunk
	const Block *getBlock(const BlockCoord& c) const
	{
		assert(c.x >= 0);
		assert(c.y >= 0);
		assert(c.z >= 0);
		assert(c.x < sizeX_);
		assert(c.y < sizeY_);
		assert(c.z < sizeZ_);

		return &blocks_[((c.z * sizeY_) + c.y) * sizeX_ + c.x];
	}

	void putBlock(const Block *b, const BlockCoord& c);

	bool isOpaque(const BlockCoord& c) const
	{
		return getBlock(c)->id != 0;
	}

	bool dirty() const { return dirty_; }
	void markDirty() { dirty_ = true; }
	void markClean() { dirty_ = false; }

protected:
	World *world_;
	Block *blocks_; // sizeX_ by sizeY_ by sizeZ_ 3D array of blocks in this chunk

	int32_t sizeX_, sizeY_, sizeZ_; // size of this chunk in blocks
	int32_t pos_x_, pos_y_, pos_z_; // position within the world of this chunk in blocks

	bool dirty_;

	void fillRectPrism(const BlockCoord& c1, const BlockCoord& c2, uint8_t block_id);
	void fillXYPlane(int32_t z, uint8_t block_id);
};

} // namespace narf

#endif // NARF_CHUNK_H
