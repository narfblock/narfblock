/*
 * NarfBlock entity class
 *
 * Copyright (c) 2013-2015 Daniel Verkamp
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

#include "narf/entity.h"
#include "narf/console.h"
#include "narf/world.h"

using namespace narf;


EntityRef::EntityRef(EntityManager& entMgr, Entity::ID id) :
	ent(entMgr.getEntityRef(id)), id(id), entMgr(entMgr) {
}


EntityRef::~EntityRef() {
	if (ent) {
		entMgr.releaseEntityRef(id);
	}
}


// TODO: move this to a subclass of entity
void explode(World* world, const BlockCoord& bc, int32_t radius) {
	Block air;
	air.id = 0;

	auto radiusSquared = radius * radius;

	for (int32_t x = 0; x < radius; x++) {
		for (int32_t y = 0; y < radius; y++) {
			for (int32_t z = 0; z < radius; z++) {
				if (x * x + y * y + z * z < radiusSquared) {
					world->putBlock(&air, {bc.x + x, bc.y + y, bc.z + z});
					world->putBlock(&air, {bc.x - x, bc.y + y, bc.z + z});
					world->putBlock(&air, {bc.x + x, bc.y - y, bc.z + z});
					world->putBlock(&air, {bc.x - x, bc.y - y, bc.z + z});
					world->putBlock(&air, {bc.x + x, bc.y + y, bc.z - z});
					world->putBlock(&air, {bc.x - x, bc.y + y, bc.z - z});
					world->putBlock(&air, {bc.x + x, bc.y - y, bc.z - z});
					world->putBlock(&air, {bc.x - x, bc.y - y, bc.z - z});
				}
			}
		}
	}
}


bool Entity::update(timediff dt)
{
	// copy previous state so we can interpolate during render
	prevPosition = position;

	bool alive = true;
	// cheesy Euler integration
	auto acceleration = Vector3f(0.0f, 0.0f, antigrav ? 0.0f : world_->getGravity());

	// TODO: once coords are converted to integers, get rid of casts of dt to double
	velocity += acceleration * (float)dt;
	position += velocity * (float)dt;

	// TODO: entity AABB should be determined based on its model
	// for now, make everything 0.75x0.75x0.75
	Vector3f center(position.x, position.y, position.z + 0.375f);
	Vector3f halfSize(0.375f, 0.375f, 0.375f);
	AABB entAABB(center, halfSize);

	// Z coord of the bottom of this entity
	float bottomZ = position.z;
	float prevBottomZ = prevPosition.z;

	// check against all blocks that could potentially intersect
	BlockCoord c(
			(int32_t)(position.x - halfSize.x),
			(int32_t)(position.y - halfSize.y),
			(int32_t)(position.z - halfSize.z));

	// size of entity aabb
	// + 1 to round up
	// + 1 since CoordIter iterates up to but not including
	int32_t sx = 2 + (int32_t)(halfSize.x * 2.0f);
	int32_t sy = 2 + (int32_t)(halfSize.y * 2.0f);
	int32_t sz = 2 + (int32_t)(halfSize.z * 2.0f);

	ZYXCoordIter<BlockCoord> iter(
		{c.x, c.y, c.z},
		{c.x + sx, c.y + sy, c.z + sz});

	bool collided = false;
	bool bounced = false;
	for (const auto& bc : iter) {
		auto block = world_->getBlock(bc);
		if (!block) {
			continue;
		}
		AABB blockAABB(world_->getBlockType(block->id)->getAABB(bc));

		if (world_->getBlock(bc)->id != 0 && blockAABB.intersect(entAABB)) {
			if (explodey) {
				explode(world_, bc, 5);
				alive = false;
				explodey = false;
				velocity.x = velocity.y = 0.0f;
			}

			if (bouncy) {
				if (!bounced) {
					position.z = ceilf(position.z) + (ceilf(position.z) - position.z);
					velocity.z = -velocity.z;
					bounced = true; // only apply bounce once...
				}
			} else {
				if (bottomZ < bc.z + 1 && prevBottomZ >= bc.z + 1) {
					// moving down
					// and the bottom of the ent is within the block below us
					// and the bottom of the ent was above the block below us in the previous tick
					if (velocity.z < 0.0f) {
						// hit a block below us while falling; zero out vertical velocity and push up
						position.z = (float)bc.z + 1.0f;
						velocity.z = 0.0f;
						onGround = true;
					}
				} else {
					collided = true;
				}
			}
		}
	}

	if (collided) {
		// for now, just teleport back to the previous position
		// TODO: better collision resolution
		// TODO: handle blocks above
		position = prevPosition;
	}

	if (!almostEqual(velocity.z, 0.0f)) {
		onGround = false;
	}

	return alive;
}

void Entity::serialize(ByteStream& s) const {
	// TODO this could be much more compactly encoded...
	// do the dumbest possible encoding that works (for now)
	s.write(position.x, LE);
	s.write(position.y, LE);
	s.write(position.z, LE);
	s.write(velocity.x, LE);
	s.write(velocity.y, LE);
	s.write(velocity.z, LE);
	s.write((uint8_t)model, LE);
}

void Entity::deserialize(ByteStream& s) {
	uint8_t tmp8;

	if (!s.read(&position.x, LE) ||
		!s.read(&position.y, LE) ||
		!s.read(&position.z, LE) ||
		!s.read(&velocity.x, LE) ||
		!s.read(&velocity.y, LE) ||
		!s.read(&velocity.z, LE) ||
		!s.read(&tmp8)) {
		narf::console->println("entity deserialize error");
		assert(0);
		return;
	}
	model = tmp8;
}


EntityIDAllocator::EntityIDAllocator() : firstFreeID_(0) {
}


EntityIDAllocator::~EntityIDAllocator() {
}


Entity::ID EntityIDAllocator::get() {
	Entity::ID id;
	if (!freeIDPool_.empty()) {
		id = freeIDPool_.back();
		freeIDPool_.pop_back();
		return id;
	} else {
		id = firstFreeID_++;
	}
	return id;
}


void EntityIDAllocator::put(Entity::ID id) {
	assert(firstFreeID_ > 0);
	if (id == firstFreeID_ - 1) {
		--firstFreeID_;
	} else {
		freeIDPool_.push_back(id);
	}
}


EntityManager::EntityManager(World* world) :
	world_(world), entityRefs_(0) {
}


Entity::ID EntityManager::newEntity() {
	assert(entityRefs_ == 0);
	if (entityRefs_ != 0) {
		narf::console->println("!!!! ERROR: newEntity() called while an EntityRef is live");
	}
	auto id = idAllocator_.get();
	entities_.emplace_back(world_, this, id);
	return id;
}


Entity::ID EntityManager::newEntity(Entity::ID id) {
	assert(entityRefs_ == 0);
	if (entityRefs_ != 0) {
		narf::console->println("!!!! ERROR: newEntity() called while an EntityRef is live");
	}

	entities_.emplace_back(world_, this, id);
	return id;
}


void EntityManager::deleteEntity(Entity::ID id) {
	assert(entityRefs_ == 0);
	if (entityRefs_ != 0) {
		console->println("!!!! ERROR: deleteEntity() called while an EntityRef is live");
	}

	for (auto entIter = std::begin(entities_); entIter != std::end(entities_); ++entIter) {
		if (entIter->id == id) {
			entities_.erase(entIter);
			idAllocator_.put(id);
			return;
		}
	}
}


Entity* EntityManager::getEntityRef(Entity::ID id) {
	// TODO: if this gets too slow, keep a sorted index of id -> Entity
	for (auto& ent : entities_) {
		if (ent.id == id) {
			entityRefs_++; // debug helper - make sure newEntity() doesn't get called while there is a live entity ref
			return &ent;
		}
	}
	return nullptr;
}


void EntityManager::releaseEntityRef(Entity::ID id) {
	entityRefs_--;
}


void EntityManager::update(timediff dt) {
	std::vector<Entity::ID> entsToDelete;
	for (auto& ent : entities_) {
		if (!ent.update(dt)) {
			entsToDelete.push_back(ent.id);
		}
	}

	for (auto& eid : entsToDelete) {
		onEntityDeleted.emit(eid);
		deleteEntity(eid);
	}
}


void EntityManager::serializeEntityFullUpdate(ByteStream& s, const Entity& ent) const {
	s.write((uint8_t)UpdateType::FullUpdate);
	s.write(ent.id, LE);
	ent.serialize(s);
}


void EntityManager::serializeEntityDelete(ByteStream& s, Entity::ID id) const {
	s.write((uint8_t)UpdateType::Deleted);
	s.write(id, LE);
}


void EntityManager::deserializeEntityUpdate(ByteStream& s) {
	Entity::ID id;
	uint8_t tmp8;

	if (!s.read(&tmp8)) { // type
		narf::console->println("entity update type deserialize error");
		assert(0);
		return;
	}

	// for now, all update types have ID next
	if (!s.read(&id, LE)) {
		narf::console->println("entity ID deserialize error");
		assert(0);
		return;
	}

	switch ((UpdateType)tmp8) {
	case UpdateType::FullUpdate:
		{
			auto ent = getEntityRef(id);
			if (!ent) {
				narf::console->println("new ent ID " + std::to_string(id));
				newEntity(id);
				ent = getEntityRef(id);
				assert(ent != nullptr);
				if (!ent) {
					return;
				}
			}

			ent->deserialize(s);
			releaseEntityRef(id);

			break;
		}

	case UpdateType::Deleted:
		narf::console->println("delete ent ID " + std::to_string(id));
		deleteEntity(id);
		break;

	default:
		narf::console->println("unknown entity update type " + std::to_string(tmp8));
		assert(0);
		break;
	}
}
