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

#ifndef NARF_PLAYERCMD_H
#define NARF_PLAYERCMD_H

#include "narf/bytestream.h"
#include "narf/math/math.h"
#include "narf/world.h"

namespace narf {

class PlayerCommand {
public:
	enum class Type {
		Invalid = 0,
		PrimaryAction = 1,
		SecondaryAction = 2,
		TernaryAction = 3,
	};

	PlayerCommand(Type type);
	PlayerCommand(ByteStreamReader& s);

	void serialize(ByteStreamWriter& s) const;

	void exec(World* world);

	Type type() const { return type_; }

	// TODO: these should be inferred from player location and orientation
	BlockCoord wbc;
	Vector3f position;
	Vector3f velocity;
	Orientationf orientation;

protected:
	Type type_;
};

} // namespace narf

#endif // NARF_PLAYERCMD_H
