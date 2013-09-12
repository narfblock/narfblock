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

void narf::World::update(double t, double dt)
{
	for (auto entp = entities_.begin(); entp != entities_.end(); ++entp) {
		auto ent = *entp;
		ent->update(t, dt);
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
		blockCoord.x = (int32_t)floor(point.x - (direction.x > 0 ? 1 : -1) * 0.0001);
		blockCoord.y = (int32_t)floor(point.y - (direction.y > 0 ? 1 : -1) * 0.0001);
		blockCoord.z = (int32_t)floor(point.z - (direction.z > 0 ? 1 : -1) * 0.0001);
		block = get_block(blockCoord.x, blockCoord.y, blockCoord.z);
		narf::BlockWrapper tmp = {block, blockCoord.x, blockCoord.y, blockCoord.z, narf::Invalid};
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

