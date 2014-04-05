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
#include <algorithm>

#include "narf/client/world.h"

void draw_cube(float x, float y, float z, uint8_t type, unsigned draw_face_mask);


void narf::client::World::put_block(const narf::Block *b, uint32_t x, uint32_t y, uint32_t z) {
	uint32_t cx, cy, cz, bx, by, bz;
	calc_chunk_coords(x, y, z, &cx, &cy, &cz, &bx, &by, &bz);
	Chunk *chunk = get_chunk(cx, cy, cz);
	chunk->put_block(b, bx, by, bz);
	chunk->rebuild_vertex_buffers();

	// update neighboring chunk meshes since they may have holes exposed by removing this block
	// or extra faces that are obstructed by adding this block
	if (bx == 0) get_chunk((cx - 1) & mask_x_, cy, cz)->rebuild_vertex_buffers();
	if (by == 0) get_chunk(cx, (cy - 1) & mask_y_, cz)->rebuild_vertex_buffers();
	if (bz == 0 && cz > 0) get_chunk(cx, cy, cz - 1)->rebuild_vertex_buffers();
	if (bx == chunk_size_x_ - 1) get_chunk((cx + 1) & mask_x_, cy, cz)->rebuild_vertex_buffers();
	if (by == chunk_size_y_ - 1) get_chunk(cx, (cy + 1) & mask_y_, cz)->rebuild_vertex_buffers();
	if (bz == chunk_size_z_ - 1 && cz < chunks_z_ - 1) get_chunk(cx, cy, cz + 1)->rebuild_vertex_buffers();
}


void narf::client::World::renderSlice(narf::gl::Texture *tiles_tex, uint32_t cx_min, uint32_t cx_max, uint32_t cy_min, uint32_t cy_max) {
	assert(cx_min <= cx_max);
	assert(cy_min <= cy_max);
	assert(cx_max <= chunks_x_);
	assert(cy_max <= chunks_y_);

	// draw chunks
	glBindTexture(narf::gl::TEXTURE_2D, tiles_tex);
	for (uint32_t cy = cy_min; cy < cy_max; cy++) {
		for (uint32_t cx = cx_min; cx < cx_max; cx++) {
			// TODO: clip any chunks that are completely out of the camera's view before calling Chunk::render()
			// TODO: clip in a sphere around the camera
			for (uint32_t cz = 0; cz < chunks_z_; cz++) {
				get_chunk(cx, cy, cz)->render();
			}
		}
	}

	// TODO: only render entities in this slice
	for (auto& ent : entities_) {
		// TODO: move this code to an Entity method
		if (ent.model) {
			// temp hack: draw an entity as a cube for physics demo
			::draw_cube(ent.position.x, ent.position.y, ent.position.z, 1, 0xFF);
		}
	}
}


void narf::client::World::render(narf::gl::Texture *tiles_tex, const narf::Camera *cam) {
	// camera
	glLoadIdentity();
	glRotatef(-(cam->orientation.pitch.toDeg() + 90.0f), 1.0f, 0.0f, 0.0f);
	glRotatef(90.0f - cam->orientation.yaw.toDeg(), 0.0f, 0.0f, 1.0f);
	glTranslatef(-cam->position.x, -cam->position.y, -cam->position.z);

	int32_t cx = (int32_t)(cam->position.x / (float)chunk_size_x_);
	int32_t cy = (int32_t)(cam->position.y / (float)chunk_size_y_);

	/*
	 * Render up to 9 slices around the camera
	 *
	 *   D  |  C  |  B
	 * -----------------
	 *   E  | mid |  A
	 * -----------------
	 *   F  |  G  |  H
	 */

	int32_t cx_min = cx - std::min(renderDistance, (int32_t)chunks_x_ - 1);
	int32_t cx_max = cx + std::min(renderDistance, (int32_t)chunks_x_ - 1) + 1;

	int32_t cy_min = cy - std::min(renderDistance, (int32_t)chunks_y_ - 1);
	int32_t cy_max = cy + std::min(renderDistance, (int32_t)chunks_y_ - 1) + 1;

	assert(cx_max >= 0);
	assert(cy_max >= 0);

	uint32_t mid_cx_min = std::max(0, cx_min);
	uint32_t mid_cy_min = std::max(0, cy_min);

	uint32_t mid_cx_max = std::min((int32_t)chunks_x_, cx_max);
	uint32_t mid_cy_max = std::min((int32_t)chunks_y_, cy_max);

	renderSlice(tiles_tex, mid_cx_min, mid_cx_max, mid_cy_min, mid_cy_max);

	if ((uint32_t)cx_max > chunks_x_) {
		uint32_t a_cx_min = 0;
		uint32_t a_cx_max = cx_max - chunks_x_;
		glPushMatrix();
		glTranslatef((float)size_x_, 0.0f, 0.0f);
		renderSlice(tiles_tex, a_cx_min, a_cx_max, mid_cy_min, mid_cy_max);
		glPopMatrix();

		if ((uint32_t)cy_max > chunks_y_) {
			uint32_t b_cy_min = 0;
			uint32_t b_cy_max = cy_max - chunks_y_;
			glPushMatrix();
			glTranslatef((float)size_x_, (float)size_y_, 0.0f);
			renderSlice(tiles_tex, a_cx_min, a_cx_max, b_cy_min, b_cy_max);
			glPopMatrix();
		}

		if (cy_min < 0) {
			uint32_t h_cy_min = cy_min + chunks_y_;
			uint32_t h_cy_max = chunks_y_;
			glPushMatrix();
			glTranslatef((float)size_x_, -(float)size_y_, 0.0f);
			renderSlice(tiles_tex, a_cx_min, a_cx_max, h_cy_min, h_cy_max);
			glPopMatrix();
		}
	}

	if (cx_min < 0) {
		uint32_t e_cx_min = cx_min + chunks_x_;
		uint32_t e_cx_max = chunks_x_;
		glPushMatrix();
		glTranslatef(-(float)size_x_, 0.0f, 0.0f);
		renderSlice(tiles_tex, e_cx_min, e_cx_max, mid_cy_min, mid_cy_max);
		glPopMatrix();

		if ((uint32_t)cy_max > chunks_y_) {
			uint32_t d_cy_min = 0;
			uint32_t d_cy_max = cy_max - chunks_y_;
			glPushMatrix();
			glTranslatef(-(float)size_x_, (float)size_y_, 0.0f);
			renderSlice(tiles_tex, e_cx_min, e_cx_max, d_cy_min, d_cy_max);
			glPopMatrix();
		}

		if (cy_min < 0) {
			uint32_t f_cy_min = cy_min + chunks_y_;
			uint32_t f_cy_max = chunks_y_;
			glPushMatrix();
			glTranslatef(-(float)size_x_, -(float)size_y_, 0.0f);
			renderSlice(tiles_tex, e_cx_min, e_cx_max, f_cy_min, f_cy_max);
			glPopMatrix();
		}
	}

	if ((uint32_t)cy_max > chunks_y_) {
		uint32_t c_cy_min = 0;
		uint32_t c_cy_max = cy_max - chunks_y_;
		glPushMatrix();
		glTranslatef(0.0f, (float)size_y_, 0.0f);
		renderSlice(tiles_tex, mid_cx_min, mid_cx_max, c_cy_min, c_cy_max);
		glPopMatrix();
	}

	if (cy_min < 0) {
		uint32_t g_cy_min = cy_min + chunks_y_;
		uint32_t g_cy_max = chunks_y_;
		glPushMatrix();
		glTranslatef(0.0f, -(float)size_y_, 0.0f);
		renderSlice(tiles_tex, mid_cx_min, mid_cx_max, g_cy_min, g_cy_max);
		glPopMatrix();
	}
}
