/*
 * NarfBlock client world class
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

#include <assert.h>

#include "narf/client/world.h"

#include <algorithm>

void draw_cube(float x, float y, float z, uint8_t type, unsigned draw_face_mask);


void narf::client::World::putBlockUnchecked(const narf::Block *b, const narf::BlockCoord& wbc) {
	ChunkCoord cc;
	narf::Chunk::BlockCoord cbc;
	calcChunkCoords(wbc, cc, cbc);
	Chunk *chunk = getChunk(cc);
	chunk->putBlock(b, cbc);
	chunk->markDirty();;

	// update neighboring chunk meshes since they may have holes exposed by removing this block
	// or extra faces that are obstructed by adding this block
	if (cbc.x == 0 && cc.x > 0) getChunk({cc.x - 1, cc.y, cc.z})->markDirty();
	if (cbc.y == 0 && cc.y > 0) getChunk({cc.x, cc.y - 1, cc.z})->markDirty();
	if (cbc.z == 0 && cc.z > 0) getChunk({cc.x, cc.y, cc.z - 1})->markDirty();
	if (cbc.x == chunkSizeX_ - 1 && cc.x < chunksX_ - 1) getChunk({cc.x + 1, cc.y, cc.z})->markDirty();
	if (cbc.y == chunkSizeY_ - 1 && cc.y < chunksY_ - 1) getChunk({cc.x, cc.y + 1, cc.z})->markDirty();
	if (cbc.z == chunkSizeZ_ - 1 && cc.z < chunksZ_ - 1) getChunk({cc.x, cc.y, cc.z + 1})->markDirty();
}


void narf::client::World::deserializeChunk(narf::ByteStreamReader& s, narf::World::ChunkCoord& cc) {
	narf::World::deserializeChunk(s, cc);
	// update neighboring chunk meshes since they may have holes exposed by updating this chunk
	// or extra faces that are obstructed by updating this chunk
	if (cc.x > 0) getChunk({cc.x - 1, cc.y, cc.z})->markDirty();
	if (cc.y > 0) getChunk({cc.x, cc.y - 1, cc.z})->markDirty();
	if (cc.z > 0) getChunk({cc.x, cc.y, cc.z - 1})->markDirty();
	if (cc.x < chunksX_ - 1) getChunk({cc.x + 1, cc.y, cc.z})->markDirty();
	if (cc.y < chunksY_ - 1) getChunk({cc.x, cc.y + 1, cc.z})->markDirty();
	if (cc.z < chunksZ_ - 1) getChunk({cc.x, cc.y, cc.z + 1})->markDirty();
}


static int32_t clampi(int32_t v, int32_t min, int32_t max) {
	if (v < min) {
		return min;
	}
	if (v > max) {
		return max;
	}
	return v;
}


void narf::client::World::render(narf::gl::Texture *tiles_tex, const narf::Camera *cam, float stateBlend) {
	// camera
	glLoadIdentity();
	glRotatef(-(cam->orientation.pitch.toDeg() + 90.0f), 1.0f, 0.0f, 0.0f);
	glRotatef(90.0f - cam->orientation.yaw.toDeg(), 0.0f, 0.0f, 1.0f);
	glTranslatef(-cam->position.x, -cam->position.y, -cam->position.z);

	// get chunk coordinates for the chunk containing the camera
	int32_t cxCam = (int32_t)(cam->position.x / (float)chunkSizeX_);
	int32_t cyCam = (int32_t)(cam->position.y / (float)chunkSizeY_);

	// calculate range of chunks to draw
	int32_t cxMin = cxCam - renderDistance;
	int32_t cxMax = cxCam + renderDistance;

	int32_t cyMin = cyCam - renderDistance;
	int32_t cyMax = cyCam + renderDistance;

	// clip chunk draw range to world size
	cxMin = clampi(cxMin, 0, (int32_t)chunksX_);
	cxMax = clampi(cxMax, 0, (int32_t)chunksX_);

	cyMin = clampi(cyMin, 0, (int32_t)chunksY_);
	cyMax = clampi(cyMax, 0, (int32_t)chunksY_);

	// draw chunks
	glBindTexture(narf::gl::TEXTURE_2D, tiles_tex);

	if (cxMin < cxMax && cyMin < cyMax) {
		for (int32_t cy = cyMin; cy < cyMax; cy++) {
			for (int32_t cx = cxMin; cx < cxMax; cx++) {
				assert(cx >= 0);
				assert(cy >= 0);
				assert(cx < chunksX_);
				assert(cy < chunksY_);
				// TODO: clip any chunks that are completely out of the camera's view before calling Chunk::render()
				// TODO: clip in a sphere around the camera
				for (int32_t cz = 0; cz < chunksZ_; cz++) {
					ChunkCoord cc(cx, cy, cz);
					getChunk(cc)->render();
				}
			}
		}
	}

	// render entities
	// TODO: move this out of world
	for (auto& ent : entities_) {
		// TODO: move this code to an Entity method
		if (ent.model) {
			// temp hack: draw an entity as a cube for physics demo
			// stateBlend represents how far (time-wise) we are between the previous state and the current state.
			// If stateBlend is in [0,1], we are interpolating between previous and current state.
			// If stateBlend is greater than 1, we are extrapolating future state. TODO: does this actually work?
			// Due to the way this interpolation works, we may be rendering up to a full tick behind the current state.
			float x = ent.position.x * stateBlend + ent.prevPosition.x * (1.0f - stateBlend);
			float y = ent.position.y * stateBlend + ent.prevPosition.y * (1.0f - stateBlend);
			float z = ent.position.z * stateBlend + ent.prevPosition.z * (1.0f - stateBlend);
			// x, y is the center of the entity; assume all entities are 1x1x1 for now
			::draw_cube(x - 0.5f, y - 0.5f, z, 1, 0xFF);
		}
	}
}
