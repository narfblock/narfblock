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


narf::World::~World() {
	for (uint32_t i = 0; i < size_x_ * size_y_ * size_z_; i++) {
		if (chunks_[i]) {
			delete chunks_[i];
		}
	}
	free(chunks_);
}


const narf::Block *narf::World::get_block(uint32_t x, uint32_t y, uint32_t z) {
	uint32_t cx, cy, cz, bx, by, bz;
	calc_chunk_coords(x, y, z, &cx, &cy, &cz, &bx, &by, &bz);
	Chunk *chunk = get_chunk(cx, cy, cz);
	return chunk->get_block(bx, by, bz);
}


void narf::World::put_block(const narf::Block *b, uint32_t x, uint32_t y, uint32_t z) {
	uint32_t cx, cy, cz, bx, by, bz;
	calc_chunk_coords(x, y, z, &cx, &cy, &cz, &bx, &by, &bz);
	Chunk *chunk = get_chunk(cx, cy, cz);
	chunk->put_block(b, bx, by, bz);
}


bool narf::World::is_opaque(uint32_t x, uint32_t y, uint32_t z) {
	uint32_t cx, cy, cz, bx, by, bz;
	calc_chunk_coords(x, y, z, &cx, &cy, &cz, &bx, &by, &bz);
	Chunk *chunk = get_chunk(cx, cy, cz);
	return chunk->is_opaque(bx, by, bz);
}


void narf::World::calc_chunk_coords(
	uint32_t x, uint32_t y, uint32_t z,
	uint32_t *chunk_x, uint32_t *chunk_y, uint32_t *chunk_z,
	uint32_t *block_x, uint32_t *block_y, uint32_t *block_z) const {

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


narf::Chunk *narf::World::new_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z) {
	return new Chunk(
		this,
		chunk_size_x_, chunk_size_y_, chunk_size_z_,
		chunk_x * chunk_size_x_, chunk_y * chunk_size_y_, chunk_z * chunk_size_z_);
}


narf::Chunk *narf::World::get_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z) {
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

narf::math::coord::Point3f narf::nextBlockIntersect(narf::math::coord::Point3f base, narf::math::Vector3f direction) {
	int8_t xDir = (direction.x > 0) ? 1 : -1;
	int8_t yDir = (direction.y > 0) ? 1 : -1;
	int8_t zDir = (direction.z > 0) ? 1 : -1;
	float distance;
	narf::math::Vector3f normal;
	narf::math::coord::Point3f nextPoint(0, 0, 0);
	auto finalPoint = base;

	narf::math::Plane<float> plane(0, 0, 0, 0);
	narf::math::coord::Point3f planePoint(0, 0, 0);

	normal = narf::math::Vector3f(1, 0, 0);
	planePoint = narf::math::coord::Point3f(xDir > 0 ? (float)floor(base.x + xDir) : (float)ceil(base.x + xDir), 0, 0);
	plane = narf::math::Plane<float>(planePoint, normal);
	nextPoint = plane.intersect(base, direction);
	finalPoint = nextPoint;
	distance = base.distanceTo(nextPoint);

	normal = narf::math::Vector3f(0, 1, 0);
	planePoint = narf::math::coord::Point3f(0, yDir > 0 ? (float)floor(base.y + yDir) : (float)ceil(base.y + yDir), 0);
	plane = narf::math::Plane<float>(planePoint, normal);
	nextPoint = plane.intersect(base, direction);
	if (base.distanceTo(nextPoint) < distance) {
		finalPoint = nextPoint;
		distance = base.distanceTo(nextPoint);
	}

	normal = narf::math::Vector3f(0, 0, 1);
	planePoint = narf::math::coord::Point3f(0, 0, zDir > 0 ? (float)floor(base.z + zDir) : (float)ceil(base.z + zDir));
	plane = narf::math::Plane<float>(planePoint, normal);
	nextPoint = plane.intersect(base, direction);
	if (base.distanceTo(nextPoint) < distance) {
		finalPoint = nextPoint;
		distance = base.distanceTo(nextPoint);
	}

	return finalPoint;
}

narf::BlockWrapper narf::World::rayTrace(narf::math::coord::Point3f basePoint, narf::math::Vector3f direction, float max_distance) {
	float distance = 0;
	const Block* block;
	bool found = false;

	narf::math::coord::Point3f prevPoint = basePoint;
	narf::math::coord::Point3f point = basePoint;

	narf::math::coord::Point3<int32_t> blockCoord(0, 0, 0);
	auto prevblockCoord = blockCoord;

	while (distance < max_distance) {
		point = narf::nextBlockIntersect(prevPoint, direction);
		blockCoord.x = (int32_t)floor(point.x - (direction.x > 0 ? 1 : -1) * 0.000000000001);
		blockCoord.y = (int32_t)floor(point.y - (direction.y > 0 ? 1 : -1) * 0.000000000001);
		blockCoord.z = (int32_t)floor(point.z - (direction.z > 0 ? 1 : -1) * 0.000000000001);
		block = get_block(blockCoord.x, blockCoord.y, blockCoord.z);
		if (block->id != 0) {
			found = true;
			break;
		}
		distance = basePoint.distanceTo(point);
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

	BlockWrapper tmp = {block, blockCoord.x, blockCoord.y, blockCoord.z, face};
	return tmp;
}

