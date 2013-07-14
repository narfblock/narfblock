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

namespace narf {

// TODO: move this to its own header
class Block {
public:
	uint8_t id;
};


class Chunk {
public:

	Chunk(uint32_t size_x, uint32_t size_y, uint32_t size_z,
	      uint32_t pos_x, uint32_t pos_y, uint32_t pos_z) :
		size_x_(size_x), size_y_(size_y), size_z_(size_z),
		pos_x_(pos_x), pos_y_(pos_y), pos_z_(pos_z)
	{
		blocks_ = (Block*)calloc(size_x_ * size_y_ * size_z_, sizeof(Block));
	}

	~Chunk()
	{
		free(blocks_);
	}

	// coordinates are relative to chunk
	const Block *get_block(uint32_t x, uint32_t y, uint32_t z) const
	{
		assert(x >= 0 && y >= 0 && z >= 0);
		assert(x < size_x_ && y < size_y_ && z < size_z_);

		return &blocks_[z * size_x_ * size_y_ + y * size_x_ + x];
	}

	void put_block(const Block *b, uint32_t x, uint32_t y, uint32_t z)
	{
		blocks_[z * size_x_ * size_y_ + y * size_x_ + x] = *b;
	}


private:
	Block *blocks_; // size_x_ by size_y_ by size_z_ 3D array of blocks in this chunk

	uint32_t size_x_, size_y_, size_z_; // size of this chunk in blocks
	uint32_t pos_x_, pos_y_, pos_z_; // position within the world of this chunk in blocks
};

} // namespace narf

#endif // NARF_CHUNK_H
