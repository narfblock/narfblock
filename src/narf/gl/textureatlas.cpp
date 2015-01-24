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
#include "narf/gl/textureatlas.h"

TextureAtlas::TextureAtlas(uint32_t width, uint32_t height, uint32_t depth) :
    width_(width), height_(height), depth_(depth),
    used_(0), id_(0) {
    // We want a one pixel border around the whole atlas to avoid any artifacts when
    // sampling texture
    Node node = {1, 1, width - 2};
    nodes_.push_back(node);

    assert((depth == 1) || (depth == 3) || (depth == 4));

    data_ = (unsigned char *)calloc(width * height * depth, sizeof(unsigned char));
    if (data_ == nullptr) {
        fprintf(stderr,
                "line %d: No more memory for allocating data\n", __LINE__ );
        exit(EXIT_FAILURE);
    }
}

TextureAtlas::~TextureAtlas() {
    if (data_) {
        free(data_);
    }
    if (id_) {
        glDeleteTextures(1, &id_);
    }
}


void TextureAtlas::setRegion(uint32_t x, uint32_t y, uint32_t regionWidth, uint32_t regionHeight, const void* newData, ptrdiff_t srcStride) {
    auto srcData = static_cast<const uint8_t*>(newData);
    auto dstData = data_;
    auto dstStride = width_ * depth_;
    auto regionStride = regionWidth * depth_;

    assert(x < width_ - 1);
    assert(x + regionWidth <= width_ - 1);
    assert(y < height_ - 1);
    assert(y + regionHeight <= height_ - 1);

    dstData += y * dstStride + x;
    for (uint32_t i = 0; i < regionHeight; i++) {
        memcpy(dstData, srcData, regionStride);
        dstData += dstStride;
        srcData += srcStride;
    }
}


uint32_t TextureAtlas::fit(size_t index, uint32_t width, uint32_t height) {
    const auto& node = nodes_[index];
    auto x = node.x;
    auto y = node.y;
    auto widthLeft = width;

    if (x + width >= width_) {
        return NoFit;
    }

    for ( ; index < nodes_.size(); index++) {
        const auto& node = nodes_[index];
        if (node.y > y) {
            y = node.y;
        }
        if (y + height > height_ - 1) {
            return NoFit;
        }
        if (node.width >= widthLeft) {
            break;
        }
        widthLeft -= node.width;
    }
    return y;
}


void TextureAtlas::merge() {
    for (size_t i = 0; i < nodes_.size() - 1; ++i) {
        auto& node = nodes_[i];
        auto& next = nodes_[i + 1];
        if (node.y == next.y) {
            node.width += next.width;
            nodes_.erase(nodes_.begin() + static_cast<long>(i) + 1);
            --i;
        }
    }
}


TextureAtlas::Region TextureAtlas::getRegion(uint32_t regionWidth, uint32_t regionHeight) {
    Region region = {0, 0, regionWidth, regionHeight};
    uint32_t bestWidth = UINT32_MAX;
    uint32_t bestHeight = UINT32_MAX;

    size_t bestIndex = SIZE_MAX;
    for (size_t i = 0; i < nodes_.size(); i++) {
        uint32_t y = fit(i, regionWidth, regionHeight);
        if (y != NoFit) {
            const auto& node = nodes_[i];
            if ((y + regionHeight < bestHeight) ||
                ((y + regionHeight == bestHeight) && (node.width < bestWidth))) {
                bestIndex = i;
                bestHeight = y + regionHeight;
                bestWidth = node.width;
                region.x = node.x;
                region.y = y;
            }
        }
    }

    if (bestIndex == SIZE_MAX) {
        // TODO: throw
        region.width = 0;
        region.height = 0;
        return region;
    }

    Node newNode;
    newNode.x = region.x;
    newNode.y = region.y + regionHeight;
    newNode.width = regionWidth;
    nodes_.insert(nodes_.begin() + static_cast<long>(bestIndex), newNode);

    for (size_t i = bestIndex + 1; i < nodes_.size(); ++i) {
        auto& node = nodes_[i];
        auto& prev = nodes_[i - 1];

        if (node.x < prev.x + prev.width) {
            uint32_t shrink = prev.x + prev.width - node.x;
            if (shrink >= node.width) {
                nodes_.erase(nodes_.begin() + static_cast<long>(i));
                --i;
            } else {
                node.x += shrink;
                node.width -= shrink;
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_),
                     0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data_);
#else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, data_);
#endif
    } else if (depth_ == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_),
                     0, GL_RGB, GL_UNSIGNED_BYTE, data_);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_),
                     0, GL_ALPHA, GL_UNSIGNED_BYTE, data_ );
    }
}
