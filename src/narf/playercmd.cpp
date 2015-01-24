/*
 * NarfBlock player commands
 *
 * Copyright (c) 2014 Daniel Verkamp
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "narf/playercmd.h"

#include "narf/console.h"

narf::PlayerCommand::PlayerCommand(Type type) : type_(type) {
}

narf::PlayerCommand::PlayerCommand(narf::ByteStreamReader& s) {
	uint16_t tmp16;
	if (!s.readLE(&tmp16)) {
		// TODO
		assert(0);
	}

	// TODO: sanity-check type
	type_ = (Type)tmp16;

	switch (type_) {
	case Type::Invalid:
		break;
	case Type::PrimaryAction:
	case Type::SecondaryAction:
		if (
			!s.readLE(&wbc.x) ||
			!s.readLE(&wbc.y) ||
			!s.readLE(&wbc.z)) {
			// TODO
			assert(0);
		}
		break;
	case Type::TernaryAction:
		position = Vector3f(s);
		velocity = Vector3f(s);
		orientation = Orientationf(s);
		break;
	}
}

void narf::PlayerCommand::serialize(narf::ByteStreamWriter& s) const {
	s.writeLE((uint16_t)type_);
	switch (type_) {
	case Type::Invalid:
		break;
	case Type::PrimaryAction:
	case Type::SecondaryAction:
		s.writeLE(wbc.x);
		s.writeLE(wbc.y);
		s.writeLE(wbc.z);
		break;
	case Type::TernaryAction:
		position.serialize(s);
		velocity.serialize(s);
		orientation.serialize(s);
	}
}

void narf::PlayerCommand::exec(narf::World* world) {
	switch (type_) {
	case Type::Invalid:
		break;
	case Type::PrimaryAction:
	case Type::SecondaryAction:
		if (type_ == Type::PrimaryAction) {
			Block b;
			b.id = 0;
			world->put_block(&b, wbc);
		} else {
			Block b;
			b.id = 5;
			world->put_block(&b, wbc);
		}
		break;
	case Type::TernaryAction:
		// fire a new entity
		auto eid = world->newEntity();
		narf::EntityRef ent(world, eid);
		ent->position = position;
		ent->prevPosition = position;
		ent->velocity = velocity + Vector3f(orientation).normalize() * 20.0f;
		ent->model = true;
		ent->bouncy = false;
		ent->explodey = true;
		break;
	}
}
