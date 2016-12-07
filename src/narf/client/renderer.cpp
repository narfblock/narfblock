/*
 * NarfBlock world renderer
 *
 * Copyright (c) 2013-2015 Daniel Verkamp
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

#include "narf/client/renderer.h"

#include "narf/camera.h"
#include "narf/world.h"

#include "narf/gl/gl.h"

using namespace narf;

static void drawQuad(gl::Buffer<BlockVertex>& vbo, const BlockTexCoord& texCoord, const float* quad, float light) {
	BlockVertex* v = vbo.reserve(4);

	memcpy(v[0].vertex, &quad[0 * 3], sizeof(v[0].vertex));
	memcpy(v[1].vertex, &quad[1 * 3], sizeof(v[1].vertex));
	memcpy(v[2].vertex, &quad[2 * 3], sizeof(v[2].vertex));
	memcpy(v[3].vertex, &quad[3 * 3], sizeof(v[3].vertex));

	v[0].texcoord[0] = texCoord.u1; v[0].texcoord[1] = texCoord.v2;
	v[1].texcoord[0] = texCoord.u2; v[1].texcoord[1] = texCoord.v2;
	v[2].texcoord[0] = texCoord.u2; v[2].texcoord[1] = texCoord.v1;
	v[3].texcoord[0] = texCoord.u1; v[3].texcoord[1] = texCoord.v1;

	v[0].color[0] = light; v[0].color[1] = light; v[0].color[2] = light;
	v[1].color[0] = light; v[1].color[1] = light; v[1].color[2] = light;
	v[2].color[0] = light; v[2].color[1] = light; v[2].color[2] = light;
	v[3].color[0] = light; v[3].color[1] = light; v[3].color[2] = light;
}


// TODO: remove me - only used for entities
static void drawCube(gl::Buffer<BlockVertex>& vbo, const Point3f& c, Vector3f& hs, const BlockType* type) {
	float x = c.x - hs.x;
	float y = c.y - hs.y;
	float z = c.z - hs.z;
	float xs = hs.x * 2.0f;
	float ys = hs.y * 2.0f;
	float zs = hs.z * 2.0f;

	const float cubeQuads[6][4*3] = {
		{x   ,y+ys,z   ,  x+xs,y+ys,z   ,  x+xs,y   ,z   ,  x   ,y   ,z   },
		{x   ,y   ,z+zs,  x+xs,y   ,z+zs,  x+xs,y+ys,z+zs,  x   ,y+ys,z+zs},
		{x+xs,y   ,z   ,  x+xs,y   ,z+zs,  x   ,y   ,z+zs,  x   ,y   ,z   },
		{x+xs,y+ys,z   ,  x   ,y+ys,z   ,  x   ,y+ys,z+zs,  x+xs,y+ys,z+zs},
		{x   ,y   ,z   ,  x   ,y   ,z+zs,  x   ,y+ys,z+zs,  x   ,y+ys,z   },
		{x+xs,y   ,z   ,  x+xs,y+ys,z   ,  x+xs,y+ys,z+zs,  x+xs,y   ,z+zs}
	};

	for (int i = 0; i < 6; i++) {
		drawQuad(vbo, type->texCoords[i], cubeQuads[i], 1.0f);
	}
}


ChunkVBO::ChunkVBO(gl::Context& gl, const ChunkCoord& cc) :
	vbo_(gl, GL_ARRAY_BUFFER, GL_STATIC_DRAW), cc_(cc), dirty_(true) {
}


ChunkVBO::~ChunkVBO() {
}


void ChunkVBO::markDirty() {
	dirty_ = true;
}


void ChunkVBO::buildVBO(World* world) {
	vbo_.clear();

	auto chunk = world->getChunk(cc_);
	if (!chunk) {
		return;
	}

	// draw blocks
	auto corner = world->calcBlockCoords(cc_);
	ZYXCoordIter<BlockCoord> iter(
		{corner.x, corner.y, corner.z},
		{corner.x + world->chunkSizeX(), corner.y + world->chunkSizeY(), corner.z + world->chunkSizeZ()});
	for (const auto& c : iter) {
		if (world->isOpaque(c)) {
			const Block *b = world->getBlock(c);

			float fx = (float)c.x, fy = (float)c.y, fz = (float)c.z;
			auto type = world->getBlockType(b->id);
			assert(type != nullptr);

			float light = fz / (float)(world->sizeZ() / 2) + 0.5f; // hax

			// don't render sides of the cube that are obscured by other opaque cubes
			if (c.y == world->sizeY() - 1 || !world->isOpaque({c.x, c.y + 1, c.z})) {
				float quad[] = {fx+1,fy+1,fz+0, fx+0,fy+1,fz+0, fx+0,fy+1,fz+1, fx+1,fy+1,fz+1};
				drawQuad(vbo_, type->texCoords[BlockFace::YPos], quad, light);
			}

			if (c.y == 0 || !world->isOpaque({c.x, c.y - 1, c.z})) {
				float quad[] = {fx+0,fy+0,fz+0, fx+1,fy+0,fz+0, fx+1,fy+0,fz+1, fx+0,fy+0,fz+1};
				drawQuad(vbo_, type->texCoords[BlockFace::YNeg], quad, light);
			}

			if (c.x == world->sizeX() - 1 || !world->isOpaque({c.x + 1, c.y, c.z})) {
				float quad[] = {fx+1,fy+0,fz+0, fx+1,fy+1,fz+0, fx+1,fy+1,fz+1, fx+1,fy+0,fz+1};
				drawQuad(vbo_, type->texCoords[BlockFace::XPos], quad, light);
			}

			if (c.x == 0 || !world->isOpaque({c.x - 1, c.y, c.z})) {
				float quad[] = {fx+0,fy+1,fz+0, fx+0,fy+0,fz+0, fx+0,fy+0,fz+1, fx+0,fy+1,fz+1};
				drawQuad(vbo_, type->texCoords[BlockFace::XNeg], quad, light);
			}

			if (c.z == world->sizeZ() - 1 || !world->isOpaque({c.x, c.y, c.z + 1})) {
				float quad[] = {fx+0,fy+0,fz+1, fx+1,fy+0,fz+1, fx+1,fy+1,fz+1, fx+0,fy+1,fz+1};
				drawQuad(vbo_, type->texCoords[BlockFace::ZPos], quad, light);
			}

			if (c.z == 0 || !world->isOpaque({c.x, c.y, c.z - 1})) {
				float quad[] = {fx+0,fy+1,fz+0, fx+1,fy+1,fz+0, fx+1,fy+0,fz+0, fx+0,fy+0,fz+0};
				drawQuad(vbo_, type->texCoords[BlockFace::ZNeg], quad, light);
			}
		}
	}

	vbo_.upload();
}


void ChunkVBO::render(World* world) {
	if (dirty_) {
		buildVBO(world);
		dirty_ = false;
	}

	if (vbo_.empty()) {
		return;
	}

	vbo_.bind();

	// TODO: move this stuff into Buffer class
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(BlockVertex), (void*)offsetof(BlockVertex, vertex));
	glTexCoordPointer(2, GL_FLOAT, sizeof(BlockVertex), (void*)offsetof(BlockVertex, texcoord));
	glColorPointer(3, GL_FLOAT, sizeof(BlockVertex), (void*)offsetof(BlockVertex, color));

	glDrawArrays(GL_QUADS, 0, (int)vbo_.count());
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	vbo_.unbind();
}


Renderer::Renderer(World* world, gl::Context& gl, gl::Texture* tilesTex) :
	wireframe(false), backfaceCulling(true), fog(true),
	world_(world), gl(gl), tilesTex_(tilesTex), entityVbo_(gl, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW) {
}


ChunkVBO* Renderer::getChunkVBO(const ChunkCoord& cc) {
	return vboCache_.get(cc);
}


void Renderer::renderChunk(const ChunkCoord& cc)
{
	auto chunkVBO = getChunkVBO(cc);
	if (chunkVBO) {
		chunkVBO->render(world_);
	} else {
		// TODO: render placeholder chunk?
		// for now, insert the chunk VBO and render it anyway
		// could do some kind of background processing so it is ready next frame?
		if (chunkRebuildCount_ > chunkRebuildLimit_) {
			return;
		}
		chunkRebuildCount_++;
		vboCache_.put(cc, ChunkVBO(gl, cc));
		chunkVBO = getChunkVBO(cc);
		chunkVBO->render(world_);
	}
}


void Renderer::markChunkDirty(const ChunkCoord& cc) {
	auto chunkVBO = getChunkVBO(cc);
	if (chunkVBO) {
		chunkVBO->markDirty();
	}
}


void Renderer::chunkUpdate(const ChunkCoord& cc) {
	markChunkDirty(cc);

	// update neighboring chunk meshes since they may have holes exposed by updating this chunk
	// or extra faces that are obstructed by updating this chunk
	markChunkDirty({cc.x - 1, cc.y, cc.z});
	markChunkDirty({cc.x, cc.y - 1, cc.z});
	markChunkDirty({cc.x, cc.y, cc.z - 1});
	markChunkDirty({cc.x + 1, cc.y, cc.z});
	markChunkDirty({cc.x, cc.y + 1, cc.z});
	markChunkDirty({cc.x, cc.y, cc.z + 1});
}


void Renderer::blockUpdate(const BlockCoord& wbc) {
	ChunkCoord cc;
	Chunk::BlockCoord cbc;
	world_->calcChunkCoords(wbc, cc, cbc);

	markChunkDirty(cc);

	// update neighboring chunk meshes since they may have holes exposed by removing this block
	// or extra faces that are obstructed by adding this block
	if (cbc.x == 0) {
		markChunkDirty({cc.x - 1, cc.y, cc.z});
	}
	if (cbc.y == 0) {
		markChunkDirty({cc.x, cc.y - 1, cc.z});
	}
	if (cbc.z == 0) {
		markChunkDirty({cc.x, cc.y, cc.z - 1});
	}
	if (cbc.x == world_->chunkSizeX() - 1) {
		markChunkDirty({cc.x + 1, cc.y, cc.z});
	}
	if (cbc.y == world_->chunkSizeY() - 1) {
		markChunkDirty({cc.x, cc.y + 1, cc.z});
	}
	if (cbc.z == world_->chunkSizeZ() - 1) {
		markChunkDirty({cc.x, cc.y, cc.z + 1});
	}
}


void Renderer::setRenderDistance(int32_t numChunks) {
	renderDistance_ = numChunks;
	size_t size = static_cast<size_t>(numChunks * 2);
	size = size * size * size;
	vboCache_.setSize(size);
}


int32_t Renderer::getRenderDistance() const {
	return renderDistance_;
}

void Renderer::render(gl::Context& context, const Camera& cam, float stateBlend) {
	render(context, cam, stateBlend, Matrix4x4f::translate(0, 0, 0));
}

void Renderer::render(gl::Context& context, const Camera& cam, float stateBlend, Matrix4x4f translate) {
	// draw 3d world and objects

	glEnable(GL_DEPTH_TEST);

	if (backfaceCulling) {
		glEnable(GL_CULL_FACE);
	}

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (fog) {
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glHint(GL_FOG_HINT, GL_DONT_CARE);
		auto renderDistanceBlocks = float(getRenderDistance() - 1) * 16.0f; // TODO - don't hardcore chunk size
		auto fogStart = std::max(renderDistanceBlocks - 48.0f, 8.0f);
		auto fogEnd = std::max(renderDistanceBlocks, 16.0f);
		glFogf(GL_FOG_START, fogStart);
		glFogf(GL_FOG_END, fogEnd);
		glEnable(GL_FOG);
	} else {
		glDisable(GL_FOG);
	}

	glEnable(GL_TEXTURE_2D);

	// viewer projection

	//float fovx = 90.0f; // degrees
	float fovy = 60.0f; // degrees
	float aspect = (float)context.width() / (float)context.height(); // TODO: include fovx in calculation
	float zNear = 0.1f;
	float zFar = 1000.0f;

	float yMax = zNear * tanf(fovy * (float)M_PI / 360.0f);
	float xMax = yMax * aspect;

	auto perspectiveMatrix = Matrix4f::frustum(-xMax, xMax, -yMax, yMax, zNear, zFar);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(perspectiveMatrix.arr);

	// camera
	auto pitchMatrix = Matrix4f::rotate((cam.orientation.pitch + (float)M_PI / 2.0f), 1.0f, 0.0f, 0.0f);
	auto yawMatrix = Matrix4f::rotate(cam.orientation.yaw - ((float)M_PI / 2.0f), 0.0f, 0.0f, 1.0f);
	auto translateMatrix = Matrix4f::translate(-cam.position.x, -cam.position.y, -cam.position.z);
	auto camMatrix = translate * pitchMatrix * yawMatrix * translateMatrix;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(camMatrix.arr);

	// get chunk coordinates for the chunk containing the camera
	int32_t cxCam = (int32_t)(cam.position.x / (float)world_->chunkSizeX());
	int32_t cyCam = (int32_t)(cam.position.y / (float)world_->chunkSizeY());

	// calculate range of chunks to draw
	int32_t cxMin = cxCam - renderDistance_;
	int32_t cxMax = cxCam + renderDistance_;

	int32_t cyMin = cyCam - renderDistance_;
	int32_t cyMax = cyCam + renderDistance_;

	// clip chunk draw range to world size
	cxMin = clampi(cxMin, 0, world_->chunksX());
	cxMax = clampi(cxMax, 0, world_->chunksX());

	cyMin = clampi(cyMin, 0, world_->chunksY());
	cyMax = clampi(cyMax, 0, world_->chunksY());

	glBindTexture(gl::TEXTURE_2D, tilesTex_);

	// draw chunks
	chunkRebuildCount_ = 0;
	if (cxMin < cxMax && cyMin < cyMax) {
		for (int32_t cy = cyMin; cy < cyMax; cy++) {
			for (int32_t cx = cxMin; cx < cxMax; cx++) {
				assert(cx >= 0);
				assert(cy >= 0);
				assert(cx < world_->chunksX());
				assert(cy < world_->chunksY());
				// TODO: clip any chunks that are completely out of the camera's view before calling Chunk::render()
				// TODO: clip in a sphere around the camera
				for (int32_t cz = 0; cz < world_->chunksZ(); cz++) {
					renderChunk({cx, cy, cz});
				}
			}
		}
	}

	// render entities
	entityVbo_.clear();

	// TODO: add accessor to world to get entity iterator
	for (auto& ent : world_->entityManager.getEntities()) {
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
			Point3f center(x, y, z);
			// TODO: entity position should be the center
			center.z += 0.375f;

			// TODO: this should be a property of entity type
			Vector3f halfSize(0.375f, 0.375f, 0.375f);

			drawCube(entityVbo_, center, halfSize, world_->getBlockType(6));
		}
	}

	entityVbo_.upload();

	entityVbo_.bind();

	// TODO: move this stuff into Buffer class
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(BlockVertex), (void*)offsetof(BlockVertex, vertex));
	glTexCoordPointer(2, GL_FLOAT, sizeof(BlockVertex), (void*)offsetof(BlockVertex, texcoord));
	glColorPointer(3, GL_FLOAT, sizeof(BlockVertex), (void*)offsetof(BlockVertex, color));

	glDrawArrays(GL_QUADS, 0, (int)entityVbo_.count());
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	entityVbo_.unbind();

	glDisable(GL_TEXTURE_2D);
}
