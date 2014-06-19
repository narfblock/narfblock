/* ============================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         http://code.google.com/p/freetype-gl/
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ============================================================================
 *
 * This source is based on the article by Jukka Jylänki :
 * "A Thousand Ways to Pack the Bin - A Practical Approach to
 * Two-Dimensional Rectangle Bin Packing", February 27, 2010.
 *
 * More precisely, this is an implementation of the Skyline Bottom-Left
 * algorithm based on C++ sources provided by Jukka Jylänki at:
 * http://clb.demon.fi/files/RectangleBinPack/
 *
 *  ============================================================================
 */
#ifndef __TEXTURE_ATLAS_H__
#define __TEXTURE_ATLAS_H__

#include <stdlib.h>

#include "vector.h"
#include "vec234.h"

/**
 * @file   texture-atlas.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup texture-atlas Texture atlas
 *
 * A texture atlas is used to pack several small regions into a single texture.
 *
 * The actual implementation is based on the article by Jukka Jylänki : "A
 * Thousand Ways to Pack the Bin - A Practical Approach to Two-Dimensional
 * Rectangle Bin Packing", February 27, 2010.
 * More precisely, this is an implementation of the Skyline Bottom-Left
 * algorithm based on C++ sources provided by Jukka Jylänki at:
 * http://clb.demon.fi/files/RectangleBinPack/
 *
 *
 * Example Usage:
 * @code
 * #include "texture-atlas.h"
 *
 * ...
 *
 * / Creates a new atlas of 512x512 with a depth of 1
 * texture_atlas_t * atlas = texture_atlas_new( 512, 512, 1 );
 *
 * // Allocates a region of 20x20
 * ivec4 region = texture_atlas_get_region( atlas, 20, 20 );
 *
 * // Fill region with some data
 * texture_atlas_set_region( atlas, region.x, region.y, region.width, region.height, data, stride )
 *
 * ...
 *
 * @endcode
 *
 * @{
 */

class TextureAtlasNode {
public:
    uint32_t x;
    uint32_t y;
    uint32_t width;
};


/**
 * A texture atlas is used to pack several small regions into a single texture.
 */

class TextureAtlas {
public:

    TextureAtlas(uint32_t width, uint32_t height, uint32_t depth);
    ~TextureAtlas();

    void upload();

    ivec4 getRegion(uint32_t width, uint32_t height);
    void setRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data, size_t stride);

    unsigned int id() const { return id_; }

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }
    uint32_t depth() const { return depth_; }

private:

    int fit(size_t index, uint32_t width, uint32_t height);
    void merge();

    /**
     * Allocated nodes
     */
    vector_t * nodes_;

    /**
     *  Width (in pixels) of the underlying texture
     */
    uint32_t width_;

    /**
     * Height (in pixels) of the underlying texture
     */
    uint32_t height_;

    /**
     * Depth (in bytes) of the underlying texture
     */
    uint32_t depth_;

    /**
     * Allocated surface size
     */
    size_t used_;

    /**
     * Texture identity (OpenGL)
     */
    unsigned int id_;

    /**
     * Atlas data
     */
    unsigned char* data_;
};

#endif /* __TEXTURE_ATLAS_H__ */
