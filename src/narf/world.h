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

#ifndef NARF_WORLD_H
#define NARF_WORLD_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <vector>
#include <functional>

#include "narf/block.h"
#include "narf/chunk.h"
#include "narf/entity.h"
#include "narf/time.h"
#include "narf/math/math.h"

namespace narf {

class World {
friend class EntityRef;
public:
	// chunk coordinate (in units of chunks) within world
	typedef Point3<int32_t> ChunkCoord;

	World(int32_t sizeX, int32_t sizeY, int32_t sizeZ, int32_t chunkSizeX, int32_t chunkSizeY, int32_t chunkSizeZ);

	virtual ~World();

	void serialize(ByteStreamWriter& s);
	void deserialize(ByteStreamReader& s);

	// TODO: make these private
	void serializeChunk(ByteStreamWriter& s, const ChunkCoord& wcc);
	virtual void deserializeChunk(ByteStreamReader& s, ChunkCoord& wcc);

	bool validCoords(const BlockCoord& wbc) const;

	const Block* getBlockUnchecked(const BlockCoord& c);
	const Block* getBlock(const BlockCoord& c);

	virtual void putBlockUnchecked(const Block* b, const BlockCoord& c);
	void putBlock(const Block* b, const BlockCoord& c);

	bool isOpaqueUnchecked(const BlockCoord& c);
	bool isOpaque(const BlockCoord& c);

	int32_t sizeX() const { return sizeX_; }
	int32_t sizeY() const { return sizeY_; }
	int32_t sizeZ() const { return sizeZ_; }

	int32_t chunksX() const { return chunksX_; }
	int32_t chunksY() const { return chunksY_; }
	int32_t chunksZ() const { return chunksZ_; }

	void setGravity(float g) { gravity_ = g; }
	float getGravity() { return gravity_; }

	static void rayTrace(Point3f basePoint, Vector3f direction,
		std::function<bool(const Point3f&, const BlockCoord&, const BlockFace&)> test);

	Entity::ID newEntity();

	size_t getNumEntities() const { return entities_.size(); }

	void update(timediff dt);

	BlockTypeId addBlockType(const BlockType &bt);
	const BlockType *getBlockType(BlockTypeId id) const;

	// TODO: make this private again
	Chunk *getChunk(const ChunkCoord& cc);

protected:

	Chunk **chunks_;

	int32_t sizeX_, sizeY_, sizeZ_; // size of the world in blocks

	int32_t chunkSizeX_, chunkSizeY_, chunkSizeZ_;

	int32_t chunksX_, chunksY_, chunksZ_; // size of the world in chunks
	int32_t chunkShiftX_, chunkShiftY_, chunkShiftZ_;
	int32_t blockMaskX_, blockMaskY_, blockMaskZ_;

	float gravity_;

	Entity::ID freeEntityID_;
	uint32_t entityRefs_;
	std::vector<narf::Entity> entities_;

	Entity* getEntityRef(Entity::ID id);
	void releaseEntityRef(Entity::ID id);

	void deleteEntity(Entity::ID id);

	void update(Entity::ID entID, double t, double dt);

	BlockTypeId numBlockTypes_;
	std::vector<BlockType> blockTypes_;

	void calcChunkCoords(const BlockCoord& wbc, ChunkCoord& cc, Chunk::BlockCoord& cbc) const;

	virtual Chunk *newChunk(int32_t chunkX, int32_t chunkY, int32_t chunkZ);

};

} // namespace narf

#endif // NARF_WORLD_H
