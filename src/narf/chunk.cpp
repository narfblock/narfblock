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
#include "narf/console.h"
#include "narf/math/coorditer.h"


narf::Chunk::Chunk(World* world, const Vector3<int32_t>& size, const ChunkCoord& pos) :
	world_(world), size_(size), pos_(pos) {
	blocks_ = (Block*)calloc(static_cast<size_t>(size_.x * size_.y * size_.z), sizeof(Block));
	posBlocks_.x = pos_.x * world->chunkSizeX();
	posBlocks_.y = pos_.y * world->chunkSizeY();
	posBlocks_.z = pos_.z * world->chunkSizeZ();
}


narf::Chunk::~Chunk() {
	free(blocks_);
}


void narf::Chunk::putBlock(const Block *b, const BlockCoord& c) {
	Block *to_replace = &blocks_[c.z * size_.x * size_.y + c.y * size_.x + c.x];
	if (!world_->getBlockType(to_replace->id)->indestructible) {
		*to_replace = *b;
		if (world_->blockUpdate) {
			world_->blockUpdate(c + posBlocks_);
		}
	}
}


void narf::Chunk::fillRectPrism(const BlockCoord& c1, const BlockCoord& c2, uint8_t block_id) {
	ZYXCoordIter<BlockCoord> iter(c1, c2);
	for (const auto& c : iter) {
		narf::Block b;
		b.id = block_id;
		putBlock(&b, c);
	}
}

void narf::Chunk::fillXYPlane(int32_t z, uint8_t block_id) {
	BlockCoord c1(0, 0, z);
	BlockCoord c2(size_.x, size_.y, z + 1);
	fillRectPrism(c1, c2, block_id);
}

void narf::Chunk::generate() {

	if (pos_ == ChunkCoord{0, 0, 3}) {
		for (int i = 0; i < 10; i++) {
			Block b;
			b.id = 7;
			putBlock(&b, {5 + i, 5, 0});
			putBlock(&b, {5, 5 + i, 0});
			putBlock(&b, {5 + i, 15, 0});
		}
	} else if (pos_ == ChunkCoord{1, 1, 3}) {
		// generate 3D "checkerboard" pattern
		for (int z = 0; z < 8; z++) {
			for (int y = 0; y < 8; y++) {
				for (int x = (y + z) & 1; x < 8; x += 2) {
					BlockCoord bc{x, y, z};
					Block b;
					b.id = 6;
					putBlock(&b, bc);
				}
			}
		}
	} else if (pos_ == ChunkCoord{2, 2, 3}) {
		// generate pyramid
		for (int z = 0; z < 7; z++) {
			auto size = 7 - z;
			fillRectPrism({z, z, z}, {z + size * 2 - 1, z + size * 2 - 1, z + 1}, 7);
		}
	} else if (pos_ == ChunkCoord{3, 3, 3}) {
		// generate some terrain from a heightmap
		static const char heightmap[16][16] = {
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0},
			{0, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 0},
			{1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 3, 1, 1, 0},
			{1, 1, 1, 2, 2, 3, 3, 3, 2, 2, 2, 3, 2, 1, 0},
			{1, 1, 1, 1, 2, 3, 4, 4, 3, 3, 3, 3, 1, 1, 1},
			{1, 1, 1, 2, 2, 3, 4, 3, 3, 3, 2, 2, 1, 1, 1},
			{0, 1, 1, 2, 2, 3, 4, 4, 3, 3, 2, 1, 1, 0, 0},
			{0, 1, 1, 1, 2, 2, 3, 3, 3, 2, 2, 2, 1, 0, 0},
			{0, 1, 1, 1, 2, 3, 4, 4, 4, 3, 2, 1, 1, 0, 0},
			{0, 1, 1, 2, 2, 2, 3, 3, 3, 2, 2, 1, 1, 0, 0},
			{1, 1, 2, 2, 2, 3, 4, 3, 3, 2, 2, 2, 2, 1, 0},
			{1, 2, 2, 2, 2, 3, 3, 3, 2, 2, 1, 2, 2, 1, 0},
			{1, 1, 2, 2, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0},
			{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}};
		for (int z = 0; z < 16; z++) {
			for (int y = 0; y < 16; y++) {
				for (int x = 0; x < 16; x++) {
					int h = heightmap[y][x];
					Block b;
					if (z < h - 1) {
						b.id = 2; // dirt
					} else if (z == h - 1) {
						b.id = 3; // dirt with grass
					} else {
						b.id = 0; // air
					}
					putBlock(&b, {x, y, z});
				}
			}
		}
	} else if (pos_ == ChunkCoord{4, 3, 3}) {
		// generate a one-block layer in the air for collision detection tests
		fillRectPrism({0, 0, 4}, {16, 16, 5}, 7);
	} else if (pos_.z == 2) {
		fillRectPrism({0, 0, 0}, {size_.x, size_.y, size_.z - 1}, 2); // dirt
		fillXYPlane(size_.z - 1, 3); // dirt with grass
	} else if (pos_.z == 0) {
		fillXYPlane(0, 1); // adminium
		fillRectPrism({0, 0, 1}, {size_.x, size_.y, size_.z}, 2); // dirt
	} else if (pos_.z < 2) {
		fillRectPrism({0, 0, 0}, {size_.x, size_.y, size_.z}, 2); // dirt
	}
}


void narf::Chunk::serialize(narf::ByteStream& s) {
	// TODO: use a CoordIter
	int32_t numBlocks = size_.x * size_.y * size_.z;
	for (int32_t i = 0; i < numBlocks; i++) {
		uint16_t tmp16 = blocks_[i].id;
		s.write(tmp16, LE);
	}
}


void narf::Chunk::deserialize(narf::ByteStream& s) {
	// TODO: add optimized array read
	int32_t numBlocks = size_.x * size_.y * size_.z;
	for (int32_t i = 0; i < numBlocks; i++) {
		uint16_t id;
		if (!s.read(&id, LE)) {
			// TODO: chunk invalid
			narf::console->println("Chunk::deserialize: ran out of blocks");
			assert(0);
			return;
		}
		blocks_[i].id = static_cast<narf::BlockTypeId>(id);
	}
	if (world_->chunkUpdate) {
		world_->chunkUpdate(pos_);
	}
}
