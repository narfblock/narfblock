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

#ifndef NARF_CLIENT_RENDERER_H
#define NARF_CLIENT_RENDERER_H

#include "narf/chunkcache.h"
#include "narf/world.h"

#include "narf/gl/gl.h"

#include <stdint.h>

namespace narf {

class Camera;

struct BlockVertex {
	GLfloat vertex[3];
	GLfloat color[3];
	GLfloat texcoord[2];
};

class ChunkVBO {
public:
	ChunkVBO(gl::Context& gl, const ChunkCoord& cc);
	~ChunkVBO();

	void render(World* world);
	void markDirty();

private:
	gl::Buffer<BlockVertex> vbo_;
	ChunkCoord cc_;
	bool dirty_;

	void drawQuad(const BlockTexCoord& texCoord, const float* quad, float light);
	void buildVBO(World* world);
};


class Renderer {
public:
	Renderer(World* world, gl::Context& gl, gl::Texture* tilesTex /*TODO*/);

	void setRenderDistance(int32_t numChunks);
	int32_t getRenderDistance() const;

	void render(gl::Context& context, const Camera& cam, float stateBlend);
	void render(gl::Context& context, const Camera& cam, float stateBlend, Matrix4x4f translate);

	void chunkUpdate(const ChunkCoord& cc);
	void blockUpdate(const BlockCoord& wbc);

	// debug options
	bool wireframe;
	bool backfaceCulling;
	bool fog;

	BlockWrapper selectedBlockFace;

private:
	World* world_;
	int32_t renderDistance_; // radius in chunks
	gl::Context& gl;
	gl::Texture* tilesTex_;
	ChunkCache<ChunkCoord, ChunkVBO> vboCache_;

	void renderChunk(const ChunkCoord& cc);
	ChunkVBO* getChunkVBO(const ChunkCoord& cc);

	void markChunkDirty(const ChunkCoord& cc);
};

} // namespace narf

#endif // NARF_CLIENT_RENDERER_H
