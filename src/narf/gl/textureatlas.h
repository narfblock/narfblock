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
#include <stdint.h>
#include <vector>

/**
 * @file
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * A texture atlas is used to pack several small regions into a single texture.
 *
 * The actual implementation is based on the article by Jukka Jylänki : "A
 * Thousand Ways to Pack the Bin - A Practical Approach to Two-Dimensional
 * Rectangle Bin Packing", February 27, 2010.
 * More precisely, this is an implementation of the Skyline Bottom-Left
 * algorithm based on C++ sources provided by Jukka Jylänki at:
 * http://clb.demon.fi/files/RectangleBinPack/
 */

/**
 * A texture atlas is used to pack several small regions into a single texture.
 */

class TextureAtlas {
public:

    TextureAtlas(uint32_t width, uint32_t height, uint32_t depth);
    ~TextureAtlas();

    void upload();

    struct Region {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
    };

    Region getRegion(uint32_t width, uint32_t height);
    void setRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data, ptrdiff_t stride);

    unsigned int id() const { return id_; }

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }
    uint32_t depth() const { return depth_; }

private:

    static const uint32_t NoFit = UINT32_MAX;
    uint32_t fit(size_t index, uint32_t width, uint32_t height);

    void merge();

    struct Node {
        uint32_t x;
        uint32_t y;
        uint32_t width;
    };

    // Allocated regions of the atlas
    std::vector<Node> nodes_;

    // Width (in pixels) of the underlying texture
    uint32_t width_;

    // Height (in pixels) of the underlying texture
    uint32_t height_;

    // Depth (in bytes) of the underlying texture
    uint32_t depth_;

    // Allocated surface size
    size_t used_;

    // OpenGL texture ID
    unsigned int id_;

    // Atlas data
    unsigned char* data_;
};

#endif /* __TEXTURE_ATLAS_H__ */
