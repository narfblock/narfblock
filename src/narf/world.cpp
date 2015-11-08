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

#include "narf/world.h"
#include "narf/console.h"

#include <float.h>


narf::World::World(int32_t sizeX, int32_t sizeY, int32_t sizeZ, int32_t chunkSizeX, int32_t chunkSizeY, int32_t chunkSizeZ) :
	entityManager(this),
	sizeX_(sizeX), sizeY_(sizeY), sizeZ_(sizeZ),
	chunkSizeX_(chunkSizeX), chunkSizeY_(chunkSizeY), chunkSizeZ_(chunkSizeZ),
	numBlockTypes_(0)
{
	// TODO: verify size is a multiple of chunk_size and a power of 2

	// chunk shifts to get chunk coords from world coords
	chunkShiftX_ = ilog2(chunkSizeX);
	chunkShiftY_ = ilog2(chunkSizeY);
	chunkShiftZ_ = ilog2(chunkSizeZ);

	// block masks to get block coords within chunk from world coords
	blockMaskX_ = (1 << chunkShiftX_) - 1;
	blockMaskY_ = (1 << chunkShiftY_) - 1;
	blockMaskZ_ = (1 << chunkShiftZ_) - 1;

	// calculate size of world in chunks
	chunksX_ = sizeX_ / chunkSizeX;
	chunksY_ = sizeY_ / chunkSizeY;
	chunksZ_ = sizeZ_ / chunkSizeZ;

	chunks_ = (Chunk**)calloc(static_cast<size_t>(chunksX_ * chunksY_ * chunksZ_), sizeof(Chunk*));

	// initialize block types
	// TODO: put this in a config file
	auto airType = BlockType(0, 0, 0, 0, 0, 0); // TODO
	airType.solid = false;
	addBlockType(airType); // air

	auto adminiumType = BlockType(4, 4, 4, 4, 4, 4);
	adminiumType.indestructible = true;
	addBlockType(adminiumType); // adminium

	addBlockType(BlockType(2, 2, 2, 2, 2, 2)); // dirt
	addBlockType(BlockType(3, 3, 3, 3, 0, 2)); // dirt with grass top
	addBlockType(BlockType(4, 4, 4, 4, 4, 4)); // TODO
	addBlockType(BlockType(5, 5, 5, 5, 5, 5)); // brick
	addBlockType(BlockType(1, 1, 1, 1, 1, 1)); // stone1
	addBlockType(BlockType(16, 16, 16, 16, 16, 16)); // stone2
	addBlockType(BlockType(17, 17, 17, 17, 17, 17)); // stone3
}


narf::World::~World() {
	for (int32_t i = 0; i < chunksX_ * chunksY_ * chunksZ_; i++) {
		if (chunks_[i]) {
			delete chunks_[i];
		}
	}
	free(chunks_);
}


bool narf::World::validCoords(const narf::BlockCoord& wbc) const {
	return
		wbc.x >= 0 &&
		wbc.y >= 0 &&
		wbc.z >= 0 &&
		wbc.x < sizeX_ &&
		wbc.y < sizeY_ &&
		wbc.z < sizeZ_;
}


const narf::Block* narf::World::getBlockUnchecked(const narf::BlockCoord& wbc) {
	ChunkCoord cc;
	narf::Chunk::BlockCoord cbc;
	calcChunkCoords(wbc, cc, cbc);
	Chunk* chunk = getChunk(cc);
	return chunk->getBlock(cbc);
}


const narf::Block* narf::World::getBlock(const narf::BlockCoord& wbc) {
	if (validCoords(wbc)) {
		return getBlockUnchecked(wbc);
	}
	return nullptr;
}


void narf::World::putBlockUnchecked(const narf::Block *b, const narf::BlockCoord& wbc) {
	ChunkCoord cc;
	narf::Chunk::BlockCoord cbc;
	calcChunkCoords(wbc, cc, cbc);
	Chunk* chunk = getChunk(cc);
	chunk->putBlock(b, cbc);
}


void narf::World::putBlock(const narf::Block *b, const narf::BlockCoord& wbc) {
	if (validCoords(wbc)) {
		putBlockUnchecked(b, wbc);
	}
}


bool narf::World::isOpaqueUnchecked(const narf::BlockCoord& wbc) {
	ChunkCoord cc;
	narf::Chunk::BlockCoord cbc;
	calcChunkCoords(wbc, cc, cbc);
	Chunk *chunk = getChunk(cc);
	return chunk->isOpaque(cbc);
}


bool narf::World::isOpaque(const narf::BlockCoord& wbc) {
	if (validCoords(wbc)) {
		return isOpaqueUnchecked(wbc);
	}
	return false;
}


void narf::World::calcChunkCoords(const narf::BlockCoord& wbc, ChunkCoord& cc, narf::Chunk::BlockCoord& cbc) const {
	assert(wbc.x >= 0);
	assert(wbc.y >= 0);
	assert(wbc.z >= 0);
	assert(wbc.x < sizeX_);
	assert(wbc.y < sizeY_);
	assert(wbc.z < sizeZ_);

	cc.x = wbc.x >> chunkShiftX_;
	cc.y = wbc.y >> chunkShiftY_;
	cc.z = wbc.z >> chunkShiftZ_;

	cbc.x = wbc.x & blockMaskX_;
	cbc.y = wbc.y & blockMaskY_;
	cbc.z = wbc.z & blockMaskZ_;
}


narf::BlockCoord narf::World::calcBlockCoords(const ChunkCoord& cc) const {
	assert(cc.x >= 0);
	assert(cc.y >= 0);
	assert(cc.z >= 0);
	assert(cc.x < chunksX_);
	assert(cc.y < chunksY_);
	assert(cc.z < chunksZ_);

	return BlockCoord{cc.x << chunkShiftX_, cc.y << chunkShiftY_, cc.z << chunkShiftZ_};
}


narf::Chunk *narf::World::newChunk(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z) {
	return new Chunk(
		this,
		Vector3<int32_t>{chunkSizeX_, chunkSizeY_, chunkSizeZ_},
		ChunkCoord{chunk_x, chunk_y, chunk_z});
}


narf::Chunk *narf::World::getChunk(const narf::ChunkCoord& wcc) {
	assert(wcc.x >= 0);
	assert(wcc.y >= 0);
	assert(wcc.z >= 0);
	assert(wcc.x < chunksX_);
	assert(wcc.y < chunksY_);
	assert(wcc.z < chunksZ_);
	Chunk *chunk = chunks_[((wcc.z * chunksY_) + wcc.y) * chunksX_ + wcc.x];
	if (!chunk) {
		// get from backing store, or allocate if it doesn't exist yet
		// for now, no backing store, so just always allocate a new chunk
		chunk = chunks_[((wcc.z * chunksY_) + wcc.y) * chunksX_ + wcc.x] =
			newChunk(wcc.x, wcc.y, wcc.z);
		chunk->generate();
	}
	return chunk;
}


void narf::World::update(narf::timediff dt) {
	entityManager.update(dt);
}


narf::BlockTypeId narf::World::addBlockType(const narf::BlockType &bt) {
	blockTypes_.push_back(bt);
	return numBlockTypes_++;
}

const narf::BlockType *narf::World::getBlockType(narf::BlockTypeId id) const {
	if (id >= numBlockTypes_) {
		// TODO
		assert(0);
		return &blockTypes_[0];
	}
	return &blockTypes_[id];
}

static narf::Point3f nextBlockIntersect(const narf::Point3f& base, const narf::Vector3f& direction, float xDir, float yDir, float zDir) {
	const narf::Plane<float> planes[] = {
		{{xDir > 0 ? (float)floor(base.x + xDir) : (float)ceil(base.x + xDir), 0, 0}, {1,0,0}},
		{{0, yDir > 0 ? (float)floor(base.y + yDir) : (float)ceil(base.y + yDir), 0}, {0,1,0}},
		{{0, 0, zDir > 0 ? (float)floor(base.z + zDir) : (float)ceil(base.z + zDir)}, {0,0,1}}
	};

	float distance = FLT_MAX;
	auto finalPoint = base;

	for (const auto& plane : planes) {
		auto nextPoint = plane.intersect(base, direction);
		if (base.distanceTo(nextPoint) < distance) {
			finalPoint = nextPoint;
			distance = base.distanceTo(nextPoint);
		}
	}

	return finalPoint;
}

void narf::World::rayTrace(narf::Point3f basePoint, narf::Vector3f direction, std::function<bool(const narf::Point3f&, const BlockCoord&, const BlockFace&)> test) {
	Point3f prevPoint = basePoint;
	BlockCoord prevBlockCoord(0, 0, 0);

	float xDir = (direction.x > 0) ? 1.0f : -1.0f;
	float yDir = (direction.y > 0) ? 1.0f : -1.0f;
	float zDir = (direction.z > 0) ? 1.0f : -1.0f;

	while (1) {
		auto point = nextBlockIntersect(prevPoint, direction, xDir, yDir, zDir);

		BlockCoord blockCoord(0, 0, 0);
		blockCoord.x = (int32_t)floor(point.x - xDir * 0.000000000001);
		blockCoord.y = (int32_t)floor(point.y - yDir * 0.000000000001);
		blockCoord.z = (int32_t)floor(point.z - zDir * 0.000000000001);

		BlockFace face = narf::BlockFace::Invalid;
		if (blockCoord.x != prevBlockCoord.x) {
			if (blockCoord.x > prevBlockCoord.x) {
				face = narf::XNeg;
			} else {
				face = narf::XPos;
			}
		} else if (blockCoord.y != prevBlockCoord.y) {
			if (blockCoord.y > prevBlockCoord.y) {
				face = narf::YNeg;
			} else {
				face = narf::YPos;
			}
		} else if (blockCoord.z != prevBlockCoord.z) {
			if (blockCoord.z > prevBlockCoord.z) {
				face = narf::ZNeg;
			} else {
				face = narf::ZPos;
			}
		}

		if (test(point, blockCoord, face)) {
			return;
		}

		prevPoint = point;
		prevBlockCoord = blockCoord;
	}
}


void narf::World::serializeChunk(ByteStream& s, const ChunkCoord& wcc) {
	s.write(wcc.x, narf::ByteStream::Endian::LITTLE);
	s.write(wcc.y, narf::ByteStream::Endian::LITTLE);
	s.write(wcc.z, narf::ByteStream::Endian::LITTLE);
	getChunk(wcc)->serialize(s);
}


void narf::World::serialize(narf::ByteStream& s) {
	s.write(sizeX_, narf::ByteStream::Endian::LITTLE);
	s.write(sizeY_, narf::ByteStream::Endian::LITTLE);
	s.write(sizeZ_, narf::ByteStream::Endian::LITTLE);
	s.write(chunkSizeX_, narf::ByteStream::Endian::LITTLE);
	s.write(chunkSizeY_, narf::ByteStream::Endian::LITTLE);
	s.write(chunkSizeZ_, narf::ByteStream::Endian::LITTLE);

	// number of serialized chunks
	s.write(chunksX_ * chunksY_ * chunksZ_, narf::ByteStream::Endian::LITTLE);

	ZYXCoordIter<ChunkCoord> iter({0, 0, 0}, {chunksX_, chunksY_, chunksZ_});
	for (const auto& wcc : iter) {
		serializeChunk(s, wcc);
	}
}

void narf::World::deserializeChunk(ByteStream& s, narf::ChunkCoord& wcc) {
	if (!s.read(&wcc.x, ByteStream::Endian::LITTLE) ||
		!s.read(&wcc.y, ByteStream::Endian::LITTLE) ||
		!s.read(&wcc.z, ByteStream::Endian::LITTLE)) {
		// TODO: chunk invalid
		narf::console->println("Chunk::deserialize: invalid");
		assert(0);
		return;
	}

	// TODO: sanity check pos

	getChunk(wcc)->deserialize(s);
 }


void narf::World::deserialize(narf::ByteStream& s) {
	if (!s.read(&sizeX_, ByteStream::Endian::LITTLE) ||
	    !s.read(&sizeY_, ByteStream::Endian::LITTLE) ||
	    !s.read(&sizeZ_, ByteStream::Endian::LITTLE) ||
	    !s.read(&chunkSizeX_, ByteStream::Endian::LITTLE) ||
	    !s.read(&chunkSizeY_, ByteStream::Endian::LITTLE) ||
	    !s.read(&chunkSizeZ_, ByteStream::Endian::LITTLE)) {
		// TODO: world invalid
		assert(0);
		return;
	}

	narf::console->println("World::deserialize: size=" + std::to_string(sizeX_) + "x" + std::to_string(sizeY_) + "x" + std::to_string(sizeZ_));

	// TODO: do other setup from ctor here

	uint32_t numChunks;
	if (!s.read(&numChunks, narf::ByteStream::Endian::LITTLE)) {
		assert(0);
		return;
	}

	while (numChunks--) {
		narf::ChunkCoord wcc;
		deserializeChunk(s, wcc);
	}
}
