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

void narf::Entity::update(double t, double dt)
{
	// cheesy Euler integration
	Vector3f acceleration = Vector3f(0.0f, 0.0f, world_->get_gravity());

	velocity += acceleration * dt;
	position += velocity * dt;

	// wrap around
	// TODO: ensure position can't go beyond one extra world size with extreme velocity
	if (position.x < 0) {
		position.x = world_->size_x() + position.x;
	} else if (position.x > world_->size_x()) {
		position.x = position.x - world_->size_x();
	}

	if (position.y < 0) {
		position.y = world_->size_y() + position.y;
	} else if (position.y > world_->size_y()) {
		position.y = position.y - world_->size_y();
	}

	// TODO: bogus collision detection
	if (position.z < 3.0f) {
		if (bouncy) {
			position.z = 6.0f - position.z;
			velocity.z = -velocity.z;
		} else {
			position.z = 3.0f;
			velocity.z = 0.0f;
		}
	}
}
