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

#include "narf/chunk.h"
#include "narf/world.h"

void narf::Chunk::put_block(const Block *b, const BlockCoord& c) {
	Block *to_replace = &blocks_[c.z * size_x_ * size_y_ + c.y * size_x_ + c.x];
	if (!world_->getBlockType(to_replace->id)->indestructible) {
		*to_replace = *b;
	}
}


void narf::Chunk::fillRectPrism(const BlockCoord& c1, const BlockCoord& c2, uint8_t block_id) {
	for (uint32_t z = c1.z; z < c2.z; z++) {
		for (uint32_t y = c1.y; y < c2.y; y++) {
			for (uint32_t x = c1.x; x < c2.x; x++) {
				narf::Block b;
				b.id = block_id;
				BlockCoord c(x, y, z);
				put_block(&b, c);
			}
		}
	}
}

void narf::Chunk::fillXYPlane(uint32_t z, uint8_t block_id) {
	BlockCoord c1(0, 0, z);
	BlockCoord c2(size_x_, size_y_, z + 1);
	fillRectPrism(c1, c2, block_id);
}

void narf::Chunk::generate() {
	if (pos_z_ == 0) {
		fillXYPlane(0, 1); // adminium
		fillRectPrism({0, 0, 1}, {size_x_, size_y_, size_z_ - 1}, 2); // dirt
		fillXYPlane(size_z_ - 1, 3); // dirt with grass
	}
}
