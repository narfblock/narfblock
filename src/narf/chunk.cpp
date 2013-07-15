/*
 * NarfBlock chunk class
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

#include "narf/chunk.h"
#include "narf/world.h"

#include "narf/gl/gl.h"


void draw_quad(uint8_t tex_id, const float *quad)
{
	uint8_t tex_x = tex_id % 16;
	uint8_t tex_y = tex_id / 16;

	float texcoord_tile_size = 1.0f / 16.0f;

	float u1 = (float)tex_x * texcoord_tile_size;
	float v1 = (float)tex_y * texcoord_tile_size;
	float u2 = u1 + texcoord_tile_size;
	float v2 = v1 + texcoord_tile_size;

	// TODO: get rid of GL_QUADS, use triangle strip?
	glBegin(GL_QUADS);

	glTexCoord2f(u1, v2);
	glVertex3fv(&quad[0 * 3]);

	glTexCoord2f(u2, v2);
	glVertex3fv(&quad[1 * 3]);

	glTexCoord2f(u2, v1);
	glVertex3fv(&quad[2 * 3]);

	glTexCoord2f(u1, v1);
	glVertex3fv(&quad[3 * 3]);

	glEnd();
}


#define DRAW_CUBE_TOP       0x01
#define DRAW_CUBE_BOTTOM    0x02
#define DRAW_CUBE_SIDE1     0x04
#define DRAW_CUBE_SIDE2     0x08
#define DRAW_CUBE_SIDE3     0x10
#define DRAW_CUBE_SIDE4     0x20


void draw_cube(float x, float y, float z, uint8_t type, unsigned draw_face_mask)
{
	const float cube_quads[][4*3] = {
		{x+0,y+0,z+0, x+1,y+0,z+0, x+1,y+1,z+0, x+0,y+1,z+0},
		{x+0,y+0,z+1, x+1,y+0,z+1, x+1,y+1,z+1, x+0,y+1,z+1},
		{x+0,y+0,z+0, x+1,y+0,z+0, x+1,y+0,z+1, x+0,y+0,z+1},
		{x+0,y+1,z+0, x+1,y+1,z+0, x+1,y+1,z+1, x+0,y+1,z+1},
		{x+0,y+0,z+0, x+0,y+0,z+1, x+0,y+1,z+1, x+0,y+1,z+0},
		{x+1,y+0,z+0, x+1,y+0,z+1, x+1,y+1,z+1, x+1,y+1,z+0}
	};

	uint8_t tex_id_top, tex_id_side, tex_id_bot;
	if (type == 2) {
		tex_id_top = 0;
		tex_id_side = 3;
		tex_id_bot = 2;
	} else {
		tex_id_top = tex_id_side = tex_id_bot = type;
	}

	if (draw_face_mask & DRAW_CUBE_SIDE4)  draw_quad(tex_id_side, cube_quads[0]);
	if (draw_face_mask & DRAW_CUBE_SIDE3)  draw_quad(tex_id_side, cube_quads[1]);
	if (draw_face_mask & DRAW_CUBE_BOTTOM) draw_quad(tex_id_bot,  cube_quads[2]);
	if (draw_face_mask & DRAW_CUBE_TOP)    draw_quad(tex_id_top,  cube_quads[3]);
	if (draw_face_mask & DRAW_CUBE_SIDE2)  draw_quad(tex_id_side, cube_quads[4]);
	if (draw_face_mask & DRAW_CUBE_SIDE1)  draw_quad(tex_id_side, cube_quads[5]);
}


void narf::Chunk::render()
{
	// precondition: tile texture atlas has already been selected by world::render()

	// draw blocks
	for (uint32_t z = 0; z < size_z_; z++) {
		for (uint32_t y = 0; y < size_y_; y++) {
			for (uint32_t x = 0; x < size_x_; x++) {
				if (is_opaque(x, y, z)) {
					const narf::Block *b = get_block(x, y, z);

					uint32_t world_x = x + pos_x_;
					uint32_t world_y = y + pos_y_;
					uint32_t world_z = z + pos_z_;

					// TODO: handle the edges separately so this can use chunk-local accesses for most cases

					// don't render sides of the cube that are obscured by other solid cubes
					unsigned mask = 0;
					if (!world_->is_opaque(world_x, world_y + 1, world_z)) mask |= DRAW_CUBE_TOP;
					if (!world_->is_opaque(world_x, world_y - 1, world_z)) mask |= DRAW_CUBE_BOTTOM;
					if (!world_->is_opaque(world_x + 1, world_y, world_z)) mask |= DRAW_CUBE_SIDE1;
					if (!world_->is_opaque(world_x - 1, world_y, world_z)) mask |= DRAW_CUBE_SIDE2;
					if (!world_->is_opaque(world_x, world_y, world_z + 1)) mask |= DRAW_CUBE_SIDE3;
					if (!world_->is_opaque(world_x, world_y, world_z - 1)) mask |= DRAW_CUBE_SIDE4;

					if (mask) draw_cube(world_x, world_y, world_z, b->id, mask);
				}
			}
		}
	}
}
