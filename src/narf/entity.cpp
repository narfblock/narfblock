/*
 * NarfBlock entity class
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

#include "narf/entity.h"
#include "narf/world.h"


narf::EntityRef::EntityRef(narf::World* world, narf::Entity::ID id) :
	ent(world->getEntityRef(id)), id(id), world(world) {
}


narf::EntityRef::~EntityRef() {
	world->releaseEntityRef(id);
}


// TODO: move this to a subclass of entity
void explode(narf::World *world, const narf::BlockCoord& bc, int32_t radius) {
	narf::Block air;
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


bool narf::Entity::update(narf::timediff dt)
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

	// check against all blocks that could potentially intersect
	BlockCoord c(
			(int32_t)(position.x - halfSize.x),
			(int32_t)(position.y - halfSize.y),
			(int32_t)(position.z - halfSize.z));

	// size of entity aabb
	// + 1 to round up
	// + 1 since CoordIter iterates up to but not including
	int32_t sx = 2u + (int32_t)(halfSize.x * 2.0f);
	int32_t sy = 2u + (int32_t)(halfSize.y * 2.0f);
	int32_t sz = 2u + (int32_t)(halfSize.z * 2.0f);

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
				// TODO: there shouldn't be any difference between these two cases
				if (bc.z < ceilf(position.z)) {
					// hit a block below us; just zero out vertical velocity and push up
					position.z = ceilf(position.z);
					velocity.z = 0.0f;
					onGround = true;
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
