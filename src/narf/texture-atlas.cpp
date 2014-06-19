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
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "narf/gl/gl.h"
#include "texture-atlas.h"

TextureAtlas::TextureAtlas(uint32_t width, uint32_t height, uint32_t depth) :
    width_(width), height_(height), depth_(depth),
    used_(0), id_(0) {
    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    TextureAtlasNode node = {1, 1, width - 2};

    assert((depth == 1) || (depth == 3) || (depth == 4));
    nodes_ = vector_new(sizeof(TextureAtlasNode));

    vector_push_back(nodes_, &node);
    data_ = (unsigned char *)calloc(width * height * depth, sizeof(unsigned char));
    if (data_ == nullptr) {
        fprintf(stderr,
                "line %d: No more memory for allocating data\n", __LINE__ );
        exit(EXIT_FAILURE);
    }
}

TextureAtlas::~TextureAtlas() {
    vector_delete(nodes_);
    if (data_) {
        free(data_);
    }
    if (id_) {
        glDeleteTextures(1, &id_);
    }
}


void TextureAtlas::setRegion(uint32_t x, uint32_t y, uint32_t regionWidth, uint32_t regionHeight, const void* newData, size_t stride) {
    auto srcData = static_cast<const uint8_t*>(newData);

    assert(x < width_ - 1);
    assert(x + regionWidth <= width_ - 1);
    assert(y < height_ - 1);
    assert(y + regionHeight <= height_ - 1);

    for (uint32_t i = 0; i < regionHeight; i++) {
        memcpy(data_ + ((y + i) * width_ + x) * depth_,
               srcData + (i * stride), regionWidth * depth_);
    }
}


int TextureAtlas::fit(size_t index, uint32_t width, uint32_t height) {
    int x, y, width_left;
    size_t i;

    auto node = (TextureAtlasNode*)vector_get(nodes_, index);
    x = node->x;
    y = node->y;
    width_left = width;
    i = index;

    if (x + width > width_ - 1 ) {
        return -1;
    }
    y = node->y;
    while (width_left > 0) {
        node = (TextureAtlasNode*)vector_get(nodes_, i);
        if (node->y > y) {
            y = node->y;
        }
        if (y + height > height_ - 1) {
            return -1;
        }
        width_left -= node->width;
        ++i;
    }
    return y;
}


void TextureAtlas::merge() {
    for (size_t i = 0; i < nodes_->size - 1; ++i) {
        auto node = (TextureAtlasNode*)vector_get(nodes_, i);
        auto next = (TextureAtlasNode*)vector_get(nodes_, i + 1);
        if (node->y == next->y) {
            node->width += next->width;
            vector_erase(nodes_, i + 1);
            --i;
        }
    }
}


ivec4 TextureAtlas::getRegion(uint32_t regionWidth, uint32_t regionHeight) {
    int y, best_height, best_width, best_index;
    ivec4 region = {{0, 0, regionWidth, regionHeight}};

    best_height = INT_MAX;
    best_index  = -1;
    best_width = INT_MAX;
    for (size_t i = 0; i < nodes_->size; i++) {
        y = fit(i, regionWidth, regionHeight);
        if (y >= 0) {
            auto node = (TextureAtlasNode *)vector_get(nodes_, i);
            if ((y + regionHeight < best_height) ||
                ((y + regionHeight == best_height) && (node->width < best_width))) {
                best_height = y + regionHeight;
                best_index = i;
                best_width = node->width;
                region.x = node->x;
                region.y = y;
            }
        }
    }

    if (best_index == -1) {
        region.x = -1;
        region.y = -1;
        region.width = 0;
        region.height = 0;
        return region;
    }

    TextureAtlasNode newNode;
    newNode.x = region.x;
    newNode.y = region.y + regionHeight;
    newNode.width = regionWidth;
    vector_insert(nodes_, best_index, &newNode);

    for (size_t i = best_index + 1; i < nodes_->size; ++i) {
        auto node = (TextureAtlasNode *)vector_get(nodes_, i);
        auto prev = (TextureAtlasNode *)vector_get(nodes_, i - 1);

        if (node->x < prev->x + prev->width) {
            int shrink = prev->x + prev->width - node->x;
            if (shrink >= node->width) {
                vector_erase(nodes_, i);
                --i;
            } else {
                node->x += shrink;
                node->width -= shrink;
                break;
            }
        } else {
            break;
        }
    }
    merge();
    used_ += regionWidth * regionHeight;
    return region;
}


void TextureAtlas::upload() {
    assert(data_);

    if (!id_) {
        glGenTextures(1, &id_);
    }

    glBindTexture(GL_TEXTURE_2D, id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if (depth_ == 4) {
#ifdef GL_UNSIGNED_INT_8_8_8_8_REV
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_,
                     0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data_);
#else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, data_);
#endif
    } else if (depth_ == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_,
                     0, GL_RGB, GL_UNSIGNED_BYTE, data_);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width_, height_,
                     0, GL_ALPHA, GL_UNSIGNED_BYTE, data_ );
    }
}
