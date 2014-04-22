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


narf::World::World(uint32_t size_x, uint32_t size_y, uint32_t size_z, uint32_t chunk_size_x, uint32_t chunk_size_y, uint32_t chunk_size_z) :
	size_x_(size_x), size_y_(size_y), size_z_(size_z),
	chunk_size_x_(chunk_size_x), chunk_size_y_(chunk_size_y), chunk_size_z_(chunk_size_z),
	entityRefs_(0),
	numBlockTypes_(0)
{
	// TODO: verify size is a multiple of chunk_size and a power of 2

	// world coordinate masks for wraparound
	// z does not wrap around
	mask_x_ = size_x_ - 1;
	mask_y_ = size_y_ - 1;

	// chunk shifts to get chunk coords from world coords
	chunk_shift_x_ = math::ilog2(chunk_size_x);
	chunk_shift_y_ = math::ilog2(chunk_size_y);
	chunk_shift_z_ = math::ilog2(chunk_size_z);

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


narf::World::~World() {
	for (uint32_t i = 0; i < size_x_ * size_y_ * size_z_; i++) {
		if (chunks_[i]) {
			delete chunks_[i];
		}
	}
	free(chunks_);
}


const narf::Block *narf::World::get_block(const World::BlockCoord& wbc) {
	ChunkCoord cc;
	narf::Chunk::BlockCoord cbc;
	calcChunkCoords(wbc, cc, cbc);
	Chunk *chunk = get_chunk(cc);
	return chunk->get_block(cbc);
}


void narf::World::put_block(const narf::Block *b, const World::BlockCoord& wbc) {
	ChunkCoord cc;
	narf::Chunk::BlockCoord cbc;
	calcChunkCoords(wbc, cc, cbc);
	Chunk *chunk = get_chunk(cc);
	chunk->put_block(b, cbc);
}


bool narf::World::is_opaque(const narf::World::BlockCoord& wbc) {
	ChunkCoord cc;
	narf::Chunk::BlockCoord cbc;
	calcChunkCoords(wbc, cc, cbc);
	Chunk *chunk = get_chunk(cc);
	return chunk->is_opaque(cbc);
}


void narf::World::calcChunkCoords(
	const narf::World::BlockCoord& wbc,
	ChunkCoord& cc,
	narf::Chunk::BlockCoord& cbc) const {

	// wrap around
	auto x = wbc.x & mask_x_;
	auto y = wbc.y & mask_y_;
	auto z = wbc.z;

	// clamp z to world height
	// TODO
	if (z >= size_z_) {
		z = size_z_ - 1;
	}

	cc.x = x >> chunk_shift_x_;
	cc.y = y >> chunk_shift_y_;
	cc.z = z >> chunk_shift_z_;

	cbc.x = x & block_mask_x_;
	cbc.y = y & block_mask_y_;
	cbc.z = z & block_mask_z_;
}


narf::Chunk *narf::World::new_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z) {
	return new Chunk(
		this,
		chunk_size_x_, chunk_size_y_, chunk_size_z_,
		chunk_x * chunk_size_x_, chunk_y * chunk_size_y_, chunk_z * chunk_size_z_);
}


narf::Chunk *narf::World::get_chunk(const narf::World::ChunkCoord& wcc) {
	auto chunk_x = wcc.x & chunk_mask_x_;
	auto chunk_y = wcc.y & chunk_mask_y_;
	auto chunk_z = wcc.z;
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


narf::Entity::ID narf::World::newEntity() {
	assert(entityRefs_ == 0);
	if (entityRefs_ != 0) {
		narf::console->println("!!!! ERROR: newEntity() called while an EntityRef is live");
	}
	auto id = freeEntityID_++;
	entities_.emplace_back(this, id);
	return id;
}


void narf::World::deleteEntity(narf::Entity::ID id) {
	assert(entityRefs_ == 0);
	if (entityRefs_ != 0) {
		narf::console->println("!!!! ERROR: deleteEntity() called while an EntityRef is live");
	}

	for (auto entIter = std::begin(entities_); entIter != std::end(entities_); ++entIter) {
		if (entIter->id == id) {
			entities_.erase(entIter);
			return;
		}
	}
}


narf::Entity* narf::World::getEntityRef(narf::Entity::ID id) {
	// debug helper - make sure newEntity() doesn't get called while there is a live entity ref
	entityRefs_++;
	// TODO: if this gets too slow, keep a sorted index of id -> Entity
	for (auto& ent : entities_) {
		if (ent.id == id) {
			return &ent;
		}
	}
	return nullptr;
}


void narf::World::releaseEntityRef(narf::Entity::ID id) {
	entityRefs_--;
}


void narf::World::update(double t, double dt) {
	std::vector<Entity::ID> entsToDelete;
	for (auto& ent : entities_) {
		if (!ent.update(t, dt)) {
			entsToDelete.push_back(ent.id);
		}
	}

	for (auto& eid : entsToDelete) {
		deleteEntity(eid);
	}
}


narf::BlockTypeId narf::World::addBlockType(const narf::BlockType &bt) {
	if (numBlockTypes_ == (sizeof(blockTypes_) / sizeof(*blockTypes_)) - 1) {
		return 0; // TODO - get a better invalid value
	}

	blockTypes_[numBlockTypes_] = bt;
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

static narf::math::coord::Point3f nextBlockIntersect(const narf::math::coord::Point3f& base, const narf::math::Vector3f& direction, float xDir, float yDir, float zDir) {
	const narf::math::Plane<float> planes[] = {
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

narf::BlockWrapper narf::World::rayTrace(narf::math::coord::Point3f basePoint, narf::math::Vector3f direction, float max_distance) {
	const Block* block;
	bool found = false;

	narf::math::coord::Point3f prevPoint = basePoint;
	narf::math::coord::Point3f point = basePoint;

	float xDir = (direction.x > 0) ? 1.0f : -1.0f;
	float yDir = (direction.y > 0) ? 1.0f : -1.0f;
	float zDir = (direction.z > 0) ? 1.0f : -1.0f;

	BlockCoord blockCoord(0, 0, 0);
	auto prevblockCoord = blockCoord;

	while (basePoint.distanceTo(point) < max_distance) {
		point = nextBlockIntersect(prevPoint, direction, xDir, yDir, zDir);
		blockCoord.x = (uint32_t)floor(point.x - xDir * 0.000000000001);
		blockCoord.y = (uint32_t)floor(point.y - yDir * 0.000000000001);
		blockCoord.z = (uint32_t)floor(point.z - zDir * 0.000000000001);
		block = get_block(blockCoord);
		if (block->id != 0) {
			found = true;
			break;
		}
		prevPoint = point;
		prevblockCoord = blockCoord;
	}

	if (!found) {
		narf::BlockWrapper tmp = {nullptr};
		return tmp;
	}

	BlockFace face = narf::Invalid;
	if (blockCoord.x != prevblockCoord.x) {
		if (blockCoord.x > prevblockCoord.x) {
			face = narf::XNeg;
		} else {
			face = narf::XPos;
		}
	} else if (blockCoord.y != prevblockCoord.y) {
		if (blockCoord.y > prevblockCoord.y) {
			face = narf::YNeg;
		} else {
			face = narf::YPos;
		}
	} else if (blockCoord.z != prevblockCoord.z) {
		if (blockCoord.z > prevblockCoord.z) {
			face = narf::ZNeg;
		} else {
			face = narf::ZPos;
		}
	}

	BlockWrapper tmp = {block, (int32_t)blockCoord.x, (int32_t)blockCoord.y, (int32_t)blockCoord.z, face};
	return tmp;
}

