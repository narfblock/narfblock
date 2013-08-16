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


// TODO: move this to a subclass of entity
void explode(narf::World *world, int32_t bx, int32_t by, int32_t bz, int32_t radius) {
	narf::Block air;
	air.id = 0;

	auto radiusSquared = radius * radius;

	for (int32_t x = 0; x < radius; x++) {
		for (int32_t y = 0; y < radius; y++) {
			for (int32_t z = 0; z < radius; z++) {
				if (x * x + y * y + z * z < radiusSquared) {
					world->put_block(&air, bx + x, by + y, bz + z);
					world->put_block(&air, bx - x, by + y, bz + z);
					world->put_block(&air, bx + x, by - y, bz + z);
					world->put_block(&air, bx - x, by - y, bz + z);
					world->put_block(&air, bx + x, by + y, bz - z);
					world->put_block(&air, bx - x, by + y, bz - z);
					world->put_block(&air, bx + x, by - y, bz - z);
					world->put_block(&air, bx - x, by - y, bz - z);
				}
			}
		}
	}
}


void narf::Entity::update(double t, double dt)
{
	// cheesy Euler integration
	auto acceleration = narf::math::Vector3f(0.0f, 0.0f, antigrav ? 0.0f : world_->get_gravity());

	velocity += acceleration * (float)dt;
	position += velocity * (float)dt;

	// wrap around
	// TODO: ensure position can't go beyond one extra world size with extreme velocity
	if (position.x < 0) {
		position.x = (float)world_->size_x() + position.x;
	} else if (position.x > world_->size_x()) {
		position.x = position.x - (float)world_->size_x();
	}

	if (position.y < 0) {
		position.y = (float)world_->size_y() + position.y;
	} else if (position.y > world_->size_y()) {
		position.y = position.y - (float)world_->size_y();
	}

	// TODO: bogus collision detection
	uint32_t bx = (uint32_t)position.x;
	uint32_t by = (uint32_t)position.y;
	uint32_t bz = (uint32_t)position.z;
	if (world_->get_block(bx, by, bz)->id != 0) {
		if (explodey) {
			explode(world_, bx, by, bz, 5);
			// TODO: delete this entity
			explodey = false;
			velocity.x = velocity.y = 0.0f;
		}

		if (bouncy) {
			position.z = ceilf(position.z) + (ceilf(position.z) - position.z);
			velocity.z = -velocity.z;
		} else {
			position.z = ceilf(position.z);
			velocity.z = 0.0f;
			onGround = true;
		}
	}

	if (!narf::math::AlmostEqual(velocity.z, 0.0f)) {
		onGround = false;
	}
}
