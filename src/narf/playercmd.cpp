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
	if (!s.readLE16(&tmp16)) {
		// TODO
		assert(0);
	}

	// TODO: sanity-check type
	type_ = (Type)tmp16;

	switch (type_) {
	case Type::PrimaryAction:
	case Type::SecondaryAction:
		if (
			!s.readLE32(&wbc_.x) ||
			!s.readLE32(&wbc_.y) ||
			!s.readLE32(&wbc_.z)) {
			// TODO
			assert(0);
		}
		break;
	}
}

void narf::PlayerCommand::serialize(narf::ByteStreamWriter& s) {
	s.writeLE16((uint16_t)type_);
	switch (type_) {
	case Type::PrimaryAction:
	case Type::SecondaryAction:
		s.writeLE32(wbc_.x);
		s.writeLE32(wbc_.y);
		s.writeLE32(wbc_.z);
		break;
	}
}

void narf::PlayerCommand::exec(narf::World* world) {
	switch (type_) {
	case Type::PrimaryAction:
	case Type::SecondaryAction:
		narf::console->println("action at " +
			std::to_string(wbc_.x) + " " +
			std::to_string(wbc_.y) + " " +
			std::to_string(wbc_.z));
		if (type_ == Type::PrimaryAction) {
			Block b;
			b.id = 0;
			world->put_block(&b, wbc_);
		} else {
			Block b;
			b.id = 5;
			world->put_block(&b, wbc_);
		}
		break;
	}
}
