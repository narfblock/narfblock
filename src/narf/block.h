/*
 * NarfBlock block class
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

#ifndef NARF_BLOCK_H
#define NARF_BLOCK_H

#include "narf/aabb.h"
#include "narf/math/math.h"

namespace narf {

// block coordinate within world
typedef Point3<uint32_t> BlockCoord;

typedef uint8_t BlockTypeId;

class Block {
public:
	BlockTypeId id;
};

enum BlockFace { XPos, XNeg, YPos, YNeg, ZPos, ZNeg, Invalid };

typedef struct {
	const Block* block;
	int32_t x;
	int32_t y;
	int32_t z;
	BlockFace face;
} BlockWrapper;


struct BlockTexCoord {
	float u1;
	float v1;
	float u2;
	float v2;
};


class BlockType {
public:
	bool solid;
	bool indestructible;

	// texture coords within tileset bitmap for each face (in BlockFace order)
	BlockTexCoord texCoords[6];

	// added to block center to get center of AABB (0,0,0 if AABB is centered on block)
	Vector3f aabbCenterOffset;

	// dimensions of bounding box relative to center
	Vector3f aabbHalfSize;

	BlockType(
		unsigned texXPos, unsigned texXNeg,
		unsigned texYPos, unsigned texYNeg,
		unsigned texZPos, unsigned texZNeg);

	// calculate AABB for a block of this type at location bc
	AABB getAABB(const BlockCoord& bc) const;
};

} // namespace narf

#endif // NARF_BLOCK_H
