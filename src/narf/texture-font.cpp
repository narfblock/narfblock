/* ===========================================================================
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

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_LCD_FILTER_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "narf/texture-font.h"
#include "narf/console.h"

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

FT_Error TextureFont::setSize(unsigned pixelSize) {
    FT_Error error;

    error = FT_Select_Charmap(face_, FT_ENCODING_UNICODE);
    if (error) {
        return error;
    }

    error = FT_Set_Pixel_Sizes(face_, pixelSize, pixelSize);
    if (error) {
        return error;
    }

    FT_Size_Metrics* metrics = &face_->size->metrics;
    ascender_ = (float)metrics->ascender / 64.0f;
    descender_ = (float)metrics->descender / 64.0f;
    height_ = (float)metrics->height / 64.0f;
    linegap_ = height_ - ascender_ + descender_;

    return 0;
}


TextureFont::Glyph::Glyph() {
    width               = 0;
    height              = 0;
    offset_x            = 0;
    offset_y            = 0;
    advance_x           = 0.0;
    advance_y           = 0.0;
    s0                  = 0.0;
    t0                  = 0.0;
    s1                  = 0.0;
    t1                  = 0.0;
}


float TextureFont::Glyph::getKerning(uint32_t prevChar) const {
    for (size_t i = 0; i < kerning.size(); ++i) {
        auto& k = kerning[i];
        if (k.charcode == charcode) {
            return k.kerning;
        }
    }
    return 0;
}


void TextureFont::generateKerning() {
    // For each glyph couple combination, check if kerning is necessary
    for (size_t i = 0; i < glyphs_.size(); i++) {
        auto& glyph = glyphs_[i];
        FT_UInt glyphIndex = FT_Get_Char_Index(face_, glyph.charcode);
        glyph.kerning.clear();
        for (size_t j = 0; j < glyphs_.size(); j++) {
            auto& prevGlyph = glyphs_[i];
            FT_UInt prevIndex = FT_Get_Char_Index(face_, prevGlyph.charcode);
            FT_Vector kerning;
            FT_Get_Kerning(face_, prevIndex, glyphIndex, FT_KERNING_UNFITTED, &kerning);
            if (kerning.x) {
                kerning_t k = {prevGlyph.charcode, (float)kerning.x};
                glyph.kerning.push_back(k);
            }
        }
    }
}


void TextureFont::printError(FT_Error error) {
    narf::console->println("FT_Error " + std::to_string(FT_Errors[error].code) + std::string(": ") + FT_Errors[error].message);
}


TextureFont::TextureFont(TextureAtlas* atlas, unsigned pixelSize, const void* memoryBase, size_t memorySize) :
    atlas_(atlas), pixelSize_(pixelSize) {
    assert(memoryBase);
    assert(memorySize);

    FT_Error error;

    library_ = nullptr;
    face_ = nullptr;
    ascender_ = descender_ = linegap_ = height_ = 0.0f;

    error = FT_Init_FreeType(&library_);
    if (error) {
        printError(error);
        // TODO: throw
        return;
    }

    error = FT_New_Memory_Face(library_, static_cast<const FT_Byte*>(memoryBase), memorySize, 0, &face_);
    if (error) {
        printError(error);
        // TODO: throw
        return;
    }

    error = setSize(pixelSize);
    if (error) {
        printError(error);
        // TODO: throw
        return;
    }
}


TextureFont::~TextureFont() {
    if (face_) {
        FT_Done_Face(face_);
    }
    if (library_) {
        FT_Done_FreeType(library_);
    }
}


TextureFont::Glyph* TextureFont::loadGlyph(uint32_t charcode) {
    assert(library_ != nullptr);
    assert(face_ != nullptr);

    FT_UInt glyphIndex = FT_Get_Char_Index(face_, charcode);

    FT_Int32 flags = 0;
    flags |= FT_LOAD_RENDER;
    flags |= FT_LOAD_FORCE_AUTOHINT;

    FT_Error error = FT_Load_Glyph(face_, glyphIndex, flags);
    if (error) {
        printError(error);
        return nullptr;
    }

    size_t w = face_->glyph->bitmap.width;
    size_t h = face_->glyph->bitmap.rows;

    // We want each glyph to be separated by at least one black pixel
    auto region = atlas_->getRegion(w + 1, h + 1);
    if (region.width == 0) {
        narf::console->println("ERROR: TextureFont atlas is full");
        // TODO: throw
        return nullptr;
    }

    uint32_t x = region.x;
    uint32_t y = region.y;
    atlas_->setRegion(x, y, w, h, face_->glyph->bitmap.buffer, face_->glyph->bitmap.pitch);

    Glyph glyph;
    glyph.charcode = charcode;
    glyph.width    = w;
    glyph.height   = h;
    glyph.offset_x = face_->glyph->bitmap_left;
    glyph.offset_y = face_->glyph->bitmap_top;
    glyph.s0       = (float)x / (float)atlas_->width();
    glyph.t0       = (float)y / (float)atlas_->height();
    glyph.s1       = (float)(x + glyph.width) / (float)atlas_->width();
    glyph.t1       = (float)(y + glyph.height) / (float)atlas_->height();

    // Discard hinting to get advance, which is in 26.6 fixed point
    FT_Load_Glyph(face_, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
    glyph.advance_x = (float)face_->glyph->advance.x / 64.0f;
    glyph.advance_y = (float)face_->glyph->advance.y / 64.0f;

    glyphs_.push_back(glyph);

    // TODO: this regenerates kerning for all glyphs, which is overkill
    // -- pass in a glyph index
    generateKerning();

    atlas_->upload();
    return &glyphs_.back();
}


TextureFont::Glyph* TextureFont::getGlyph(uint32_t charcode) {
    // Check if charcode has already been loaded
    for (size_t i = 0; i < glyphs_.size(); ++i) {
        if (glyphs_[i].charcode == charcode) {
            return &glyphs_[i];
        }
    }

    // Glyph has not been loaded yet
    return loadGlyph(charcode);
}
