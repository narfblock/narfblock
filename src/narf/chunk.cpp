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

void narf::Chunk::put_block(const Block *b, uint32_t x, uint32_t y, uint32_t z) {
	Block *to_replace = &blocks_[z * size_x_ * size_y_ + y * size_x_ + x];
	if (!world_->getBlockType(to_replace->id)->indestructible) {
		*to_replace = *b;
	}
}


void narf::Chunk::fillRectPrism(uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2, uint32_t z1, uint32_t z2, uint8_t block_id) {
	for (uint32_t z = z1; z < z2; z++) {
		for (uint32_t y = y1; y < y2; y++) {
			for (uint32_t x = x1; x < x2; x++) {
				narf::Block b;
				b.id = block_id;
				put_block(&b, x, y, z);
			}
		}
	}
}

void narf::Chunk::fillPlane(uint32_t z, uint8_t block_id) {
	fillRectPrism(0, size_x_, 0, size_y_, z, z + 1, block_id);
}

void narf::Chunk::generate() {
	if (pos_z_ == 0) {
		fillPlane(0, 1); // adminium
		fillRectPrism(0, size_x_, 0, size_y_, 1, size_z_ - 1, 2); // dirt
		fillPlane(size_z_ - 1, 3); // dirt with grass
	}
}
