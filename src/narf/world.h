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
	// block coordinate within world
	typedef math::coord::Point3<uint32_t> BlockCoord;

	// chunk coordinate (in units of chunks) within world
	typedef math::coord::Point3<uint32_t> ChunkCoord;

	World(uint32_t size_x, uint32_t size_y, uint32_t size_z, uint32_t chunk_size_x, uint32_t chunk_size_y, uint32_t chunk_size_z);

	virtual ~World();

	void serialize(ByteStreamWriter& s);
	void deserialize(ByteStreamReader& s);

	// TODO: make these private
	void serializeChunk(ByteStreamWriter& s, const ChunkCoord& wcc);
	virtual void deserializeChunk(ByteStreamReader& s, ChunkCoord& wcc);

	const Block *get_block(const BlockCoord& c);
	virtual void put_block(const Block *b, const BlockCoord& c);

	bool is_opaque(const BlockCoord& c);

	uint32_t size_x() const { return size_x_; }
	uint32_t size_y() const { return size_y_; }
	uint32_t size_z() const { return size_z_; }

	uint32_t chunks_x() const { return chunks_x_; }
	uint32_t chunks_y() const { return chunks_y_; }
	uint32_t chunks_z() const { return chunks_z_; }

	void set_gravity(float g) { gravity_ = g; }
	float get_gravity() { return gravity_; }

	static void rayTrace(narf::math::coord::Point3f basePoint, narf::math::Vector3f direction,
		std::function<bool(const narf::math::coord::Point3f&, const BlockCoord&, const BlockFace&)> test);

	Entity::ID newEntity();

	size_t getNumEntities() const { return entities_.size(); }

	void update(timediff dt);

	BlockTypeId addBlockType(const BlockType &bt);
	const BlockType *getBlockType(BlockTypeId id) const;

	// TODO: make this private again
	Chunk *get_chunk(const ChunkCoord& cc);

protected:

	Chunk **chunks_;

	uint32_t size_x_, size_y_, size_z_; // size of the world in blocks
	uint32_t mask_x_, mask_y_;

	uint32_t chunk_size_x_, chunk_size_y_, chunk_size_z_;

	uint32_t chunks_x_, chunks_y_, chunks_z_; // size of the world in chunks
	uint32_t chunk_mask_x_, chunk_mask_y_;
	uint32_t chunk_shift_x_, chunk_shift_y_, chunk_shift_z_;
	uint32_t block_mask_x_, block_mask_y_, block_mask_z_;

	float gravity_;

	Entity::ID freeEntityID_;
	uint32_t entityRefs_;
	std::vector<narf::Entity> entities_;


	Entity* getEntityRef(Entity::ID id);
	void releaseEntityRef(Entity::ID id);

	void deleteEntity(Entity::ID id);

	void update(Entity::ID entID, double t, double dt);

	BlockTypeId numBlockTypes_;
	BlockType blockTypes_[256];

	void calcChunkCoords(
		const World::BlockCoord& wbc,
		ChunkCoord& cc,
		Chunk::BlockCoord& cbc) const;

	virtual Chunk *new_chunk(uint32_t chunk_x, uint32_t chunk_y, uint32_t chunk_z);

};

} // namespace narf

#endif // NARF_WORLD_H
