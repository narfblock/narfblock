/*
 * NarfBlock client chunk class
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

#include "narf/client/chunk.h"
#include "narf/client/world.h"

#include "narf/gl/gl.h"

// TODO: remove me - only used for bouncy block
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




// TODO: remove me - only used for bouncy block
void draw_cube(float x, float y, float z, uint8_t type, unsigned draw_face_mask)
{
	const float cube_quads[][4*3] = {
		{x+0,y+1,z+0, x+1,y+1,z+0, x+1,y+0,z+0, x+0,y+0,z+0},
		{x+0,y+0,z+1, x+1,y+0,z+1, x+1,y+1,z+1, x+0,y+1,z+1},
		{x+1,y+0,z+0, x+1,y+0,z+1, x+0,y+0,z+1, x+0,y+0,z+0},
		{x+1,y+1,z+0, x+0,y+1,z+0, x+0,y+1,z+1, x+1,y+1,z+1},
		{x+0,y+0,z+0, x+0,y+0,z+1, x+0,y+1,z+1, x+0,y+1,z+0},
		{x+1,y+0,z+0, x+1,y+1,z+0, x+1,y+1,z+1, x+1,y+0,z+1}
	};

	uint8_t tex_id_top, tex_id_side, tex_id_bot;
	if (type == 2) {
		tex_id_top = 0;
		tex_id_side = 3;
		tex_id_bot = 2;
	} else {
		tex_id_top = tex_id_side = tex_id_bot = type;
	}

	draw_quad(tex_id_side, cube_quads[0]);
	draw_quad(tex_id_side, cube_quads[1]);
	draw_quad(tex_id_bot,  cube_quads[2]);
	draw_quad(tex_id_top,  cube_quads[3]);
	draw_quad(tex_id_side, cube_quads[4]);
	draw_quad(tex_id_side, cube_quads[5]);
}



void narf::client::Chunk::draw_quad(narf::gl::Buffer<BlockVertex> &vbo, const narf::BlockTexCoord &texCoord, const float *quad)
{
	float light = quad[2] / (float)(world_->size_z() / 2) + 0.5f; // hax

	BlockVertex v[4];

	memcpy(v[0].vertex, &quad[0*3], sizeof(v[0].vertex));
	memcpy(v[1].vertex, &quad[1*3], sizeof(v[1].vertex));
	memcpy(v[2].vertex, &quad[2*3], sizeof(v[2].vertex));
	memcpy(v[3].vertex, &quad[3*3], sizeof(v[3].vertex));

	v[0].texcoord[0] = texCoord.u1; v[0].texcoord[1] = texCoord.v2;
	v[1].texcoord[0] = texCoord.u2; v[1].texcoord[1] = texCoord.v2;
	v[2].texcoord[0] = texCoord.u2; v[2].texcoord[1] = texCoord.v1;
	v[3].texcoord[0] = texCoord.u1; v[3].texcoord[1] = texCoord.v1;

	v[0].color[0] = light; v[0].color[1] = light; v[0].color[2] = light;
	v[1].color[0] = light; v[1].color[1] = light; v[1].color[2] = light;
	v[2].color[0] = light; v[2].color[1] = light; v[2].color[2] = light;
	v[3].color[0] = light; v[3].color[1] = light; v[3].color[2] = light;

	vbo.append(v[0]);
	vbo.append(v[1]);
	vbo.append(v[2]);
	vbo.append(v[3]);
}


void narf::client::Chunk::build_vertex_buffers()
{
	vbo_.clear();

	// draw blocks
	math::coord::ZYXCoordIter<BlockCoord> iter({0, 0, 0}, {size_x_, size_y_, size_z_});
	for (const auto& c : iter) {
		if (is_opaque(c)) {
			const narf::Block *b = get_block(c);

			uint32_t world_x = c.x + pos_x_;
			uint32_t world_y = c.y + pos_y_;
			uint32_t world_z = c.z + pos_z_;

			// TODO: handle the edges separately so this can use chunk-local accesses for most cases

			float fx = (float)world_x, fy = (float)world_y, fz = (float)world_z;

			auto type = world_->getBlockType(b->id);
			assert(type != nullptr);

			// don't render sides of the cube that are obscured by other opaque cubes
			if (!world_->is_opaque({world_x, world_y + 1, world_z})) {
				float quad[] = {fx+1,fy+1,fz+0, fx+0,fy+1,fz+0, fx+0,fy+1,fz+1, fx+1,fy+1,fz+1};
				draw_quad(vbo_, type->texCoords[BlockFace::YPos], quad);
			}

			if (!world_->is_opaque({world_x, world_y - 1, world_z})) {
				float quad[] = {fx+0,fy+0,fz+0, fx+1,fy+0,fz+0, fx+1,fy+0,fz+1, fx+0,fy+0,fz+1};
				draw_quad(vbo_, type->texCoords[BlockFace::YNeg], quad);
			}

			if (!world_->is_opaque({world_x + 1, world_y, world_z})) {
				float quad[] = {fx+1,fy+0,fz+0, fx+1,fy+1,fz+0, fx+1,fy+1,fz+1, fx+1,fy+0,fz+1};
				draw_quad(vbo_, type->texCoords[BlockFace::XPos], quad);
			}

			if (!world_->is_opaque({world_x - 1, world_y, world_z})) {
				float quad[] = {fx+0,fy+1,fz+0, fx+0,fy+0,fz+0, fx+0,fy+0,fz+1, fx+0,fy+1,fz+1};
				draw_quad(vbo_, type->texCoords[BlockFace::XNeg], quad);
			}

			if (world_z == world_->size_z() - 1 || !world_->is_opaque({world_x, world_y, world_z + 1})) {
				float quad[] = {fx+0,fy+0,fz+1, fx+1,fy+0,fz+1, fx+1,fy+1,fz+1, fx+0,fy+1,fz+1};
				draw_quad(vbo_, type->texCoords[BlockFace::ZPos], quad);
			}

			if (world_z != 0 && !world_->is_opaque({world_x, world_y, world_z - 1})) {
				float quad[] = {fx+0,fy+1,fz+0, fx+1,fy+1,fz+0, fx+1,fy+0,fz+0, fx+0,fy+0,fz+0};
				draw_quad(vbo_, type->texCoords[BlockFace::ZNeg], quad);
			}
		}
	}

	vbo_.upload();
}


// TODO: rework gl::Buffer so it can do this
void draw_vbo(narf::gl::Buffer<narf::client::BlockVertex> &vbo)
{
	if (vbo.empty()) {
		return;
	}

	vbo.bind();

	// TODO: move this stuff into Buffer class
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(narf::client::BlockVertex), (void*)offsetof(narf::client::BlockVertex, vertex));
	glTexCoordPointer(2, GL_FLOAT, sizeof(narf::client::BlockVertex), (void*)offsetof(narf::client::BlockVertex, texcoord));
	glColorPointer(3, GL_FLOAT, sizeof(narf::client::BlockVertex), (void*)offsetof(narf::client::BlockVertex, color));

	glDrawArrays(GL_QUADS, 0, (int)vbo.count());
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	vbo.unbind();
}


void narf::client::Chunk::render()
{
	// precondition: tile texture atlas has already been selected by world::render()

	if (rebuild_vertex_buffers_) {
		build_vertex_buffers();
		rebuild_vertex_buffers_ = false;
	}

	draw_vbo(vbo_);
}

void narf::client::Chunk::deserialize(ByteStreamReader& s) {
	narf::Chunk::deserialize(s);
	rebuild_vertex_buffers();
}
