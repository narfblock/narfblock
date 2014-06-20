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
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "texture-font.h"

#define HRES  64
#define HRESf 64.f
#define DPI   72

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

int TextureFont::loadFace(float size, FT_Library* library, FT_Face* face) {
    FT_Error error;
    FT_Matrix matrix = {
        (int)((1.0/HRES) * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((1.0)      * 0x10000L)};

    assert(library);
    assert(size);

    /* Initialize library */
    error = FT_Init_FreeType(library);
    if (error) {
        fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                FT_Errors[error].code, FT_Errors[error].message);
        return 0;
    }

    /* Load face */
    switch (location_) {
    case Location::File:
        error = FT_New_Face(*library, filename_, 0, face);
        break;

    case Location::Memory:
        error = FT_New_Memory_Face(*library,
            static_cast<const FT_Byte*>(memory_.base), memory_.size, 0, face);
        break;
    }

    if (error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        FT_Done_FreeType(*library);
        return 0;
    }

    /* Select charmap */
    error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
    if (error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        FT_Done_Face(*face);
        FT_Done_FreeType(*library);
        return 0;
    }

    /* Set char size */
    error = FT_Set_Char_Size(*face, (int)(size * HRES), 0, DPI * HRES, DPI);

    if (error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        FT_Done_Face(*face);
        FT_Done_FreeType(*library);
        return 0;
    }

    /* Set transform matrix */
    FT_Set_Transform(*face, &matrix, NULL);

    return 1;
}


int TextureFont::getFaceWithSize(float size, FT_Library* library, FT_Face* face) {
    return loadFace(size, library, face);
}



int TextureFont::getFace(FT_Library* library, FT_Face* face) {
    return getFaceWithSize(size_, library, face);
}


int TextureFont::getHiResFace(FT_Library* library, FT_Face* face) {
    return getFaceWithSize(size_ * 100.f, library, face);
}


TextureFont::Glyph::Glyph() {
    id                  = 0;
    width               = 0;
    height              = 0;
    outline_type        = 0;
    outline_thickness   = 0.0;
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
    FT_Library library;
    FT_Face face;
    FT_UInt glyph_index, prev_index;
    FT_Vector kerning;

    /* Load font */
    if (!getFace(&library, &face)) {
        return;
    }

    /* For each glyph couple combination, check if kerning is necessary */
    /* Starts at index 1 since 0 is for the special backgroudn glyph */
    for (size_t i = 1; i < glyphs_.size(); ++i) {
        auto& glyph = glyphs_[i];
        glyph_index = FT_Get_Char_Index(face, glyph.charcode);
        glyph.kerning.clear();

        for (size_t j = 1; j < glyphs_.size(); ++j) {
            auto& prevGlyph = glyphs_[i];
            prev_index = FT_Get_Char_Index(face, prevGlyph.charcode);
            FT_Get_Kerning(face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning);
            // printf("%c(%d)-%c(%d): %ld\n",
            //       prev_glyph->charcode, prev_glyph->charcode,
            //       glyph_index, glyph_index, kerning.x);
            if (kerning.x) {
                kerning_t k = {prevGlyph.charcode, kerning.x / (float)(HRESf*HRESf)};
                glyph.kerning.push_back(k);
            }
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}


int TextureFont::init() {
    FT_Library library;
    FT_Face face;
    FT_Size_Metrics metrics;

    assert(atlas_);
    assert(size_ > 0);
    assert((location_ == Location::File && filename_)
        || (location_ == Location::Memory
            && memory_.base && memory_.size));

    height_ = 0;
    ascender_ = 0;
    descender_ = 0;
    outlineType_ = 0;
    outlineThickness_ = 0.0;
    hinting_ = 1;
    kerning_ = 1;
    filtering_ = 1;

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    lcdWeights_[0] = 0x10;
    lcdWeights_[1] = 0x40;
    lcdWeights_[2] = 0x70;
    lcdWeights_[3] = 0x40;
    lcdWeights_[4] = 0x10;

    /* Get font metrics at high resolution */
    if (!getHiResFace(&library, &face)) {
        return -1;
    }

    underlinePosition_ = underlinePosition_ / (float)(HRESf*HRESf) * size_;
    underlinePosition_ = round(underlinePosition_);
    if (underlinePosition_ > -2) {
        underlinePosition_ = -2.0;
    }

    underlineThickness_ = underlineThickness_ / (float)(HRESf*HRESf) * size_;
    underlineThickness_ = round(underlineThickness_);
    if (underlineThickness_ < 1) {
        underlineThickness_ = 1.0;
    }

    metrics = face->size->metrics;
    ascender_ = (metrics.ascender >> 6) / 100.0;
    descender_ = (metrics.descender >> 6) / 100.0;
    height_ = (metrics.height >> 6) / 100.0;
    linegap_ = height_ - ascender_ + descender_;
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    /* -1 is a special glyph */
    getGlyph(-1);
    return 0;
}


TextureFont::TextureFont(TextureAtlas* atlas, float ptSize, const char* filename) {
    assert(filename);

    atlas_ = atlas;
    size_  = ptSize;

    location_ = Location::File;
    filename_ = strdup(filename);

    if (init()) {
        // TODO
        //texture_font_delete(self);
        //return NULL;
    }
}


TextureFont::TextureFont(TextureAtlas* atlas, float ptSize, const void* memoryBase, size_t memorySize) {
    assert(memoryBase);
    assert(memorySize);

    atlas_ = atlas;
    size_  = ptSize;

    location_ = Location::Memory;
    memory_.base = memoryBase;
    memory_.size = memorySize;

    if (init()) {
        // TODO
        //texture_font_delete(self);
        //return NULL;
    }
}


TextureFont::~TextureFont() {
    if (location_ == Location::File && filename_) {
        free(filename_);
    }
}


size_t TextureFont::loadGlyph(uint32_t charcode) {
    size_t x, y, width, height, depth, w, h;
    FT_Library library;
    FT_Error error;
    FT_Face face;
    FT_Glyph ft_glyph;
    FT_GlyphSlot slot;
    FT_Bitmap ft_bitmap;

    FT_UInt glyph_index;
    FT_Int32 flags = 0;
    int ft_glyph_top = 0;
    int ft_glyph_left = 0;

    size_t missed = 0;

    /* Check if charcode has been already loaded */
    for (size_t j = 0; j < glyphs_.size(); ++j) {
        auto& glyph = glyphs_[j];
        // If charcode is -1, we don't care about outline type or thickness
        if ((glyph.charcode == charcode) &&
            ((charcode == (uint32_t)(-1)) ||
            ((glyph.outline_type == outlineType_) &&
             (glyph.outline_thickness == outlineThickness_)))) {
            return 0;
        }
    }

    width  = atlas_->width();
    height = atlas_->height();
    depth  = atlas_->depth();

    if (!getFace(&library, &face)) {
        return 1;
    }

    flags = 0;
    ft_glyph_top = 0;
    ft_glyph_left = 0;
    glyph_index = FT_Get_Char_Index(face, charcode);
    // WARNING: We use texture-atlas depth to guess if user wants
    //          LCD subpixel rendering

    if (outlineType_ > 0) {
        flags |= FT_LOAD_NO_BITMAP;
    } else {
        flags |= FT_LOAD_RENDER;
    }

    if (!hinting_) {
        flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
    } else {
        flags |= FT_LOAD_FORCE_AUTOHINT;
    }


    if (depth == 3) {
        FT_Library_SetLcdFilter(library, FT_LCD_FILTER_LIGHT);
        flags |= FT_LOAD_TARGET_LCD;
        if (filtering_) {
            FT_Library_SetLcdFilterWeights(library, lcdWeights_);
        }
    }
    error = FT_Load_Glyph(face, glyph_index, flags);
    if (error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return 0;
    }


    if (outlineType_ == 0) {
        slot            = face->glyph;
        ft_bitmap       = slot->bitmap;
        ft_glyph_top    = slot->bitmap_top;
        ft_glyph_left   = slot->bitmap_left;
    } else {
        FT_Stroker stroker;
        FT_BitmapGlyph ft_bitmap_glyph;
        error = FT_Stroker_New(library, &stroker);
        if (error) {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                    FT_Errors[error].code, FT_Errors[error].message);
            FT_Done_Face(face);
            FT_Stroker_Done(stroker);
            FT_Done_FreeType(library);
            return 0;
        }
        FT_Stroker_Set(stroker,
                        (int)(outlineThickness_ * HRES),
                        FT_STROKER_LINECAP_ROUND,
                        FT_STROKER_LINEJOIN_ROUND,
                        0);
        error = FT_Get_Glyph(face->glyph, &ft_glyph);
        if (error) {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                    FT_Errors[error].code, FT_Errors[error].message);
            FT_Done_Face(face);
            FT_Stroker_Done(stroker);
            FT_Done_FreeType(library);
            return 0;
        }

        if (outlineType_ == 1) {
            error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
        } else if (outlineType_ == 2) {
            error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
        } else if (outlineType_ == 3) {
            error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 1, 1 );
        }

        if (error) {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                    FT_Errors[error].code, FT_Errors[error].message);
            FT_Done_Face(face);
            FT_Stroker_Done(stroker);
            FT_Done_FreeType(library);
            return 0;
        }

        if (depth == 1) {
            error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
            if (error) {
                fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                        FT_Errors[error].code, FT_Errors[error].message);
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return 0;
            }
        } else {
            error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
            if (error) {
                fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                        FT_Errors[error].code, FT_Errors[error].message);
                FT_Done_Face(face);
                FT_Stroker_Done(stroker);
                FT_Done_FreeType(library);
                return 0;
            }
        }
        ft_bitmap_glyph = (FT_BitmapGlyph)ft_glyph;
        ft_bitmap       = ft_bitmap_glyph->bitmap;
        ft_glyph_top    = ft_bitmap_glyph->top;
        ft_glyph_left   = ft_bitmap_glyph->left;
        FT_Stroker_Done(stroker);
    }


    // We want each glyph to be separated by at least one black pixel
    // (for example for shader used in demo-subpixel.c)
    w = ft_bitmap.width/depth + 1;
    h = ft_bitmap.rows + 1;
    auto region = atlas_->getRegion(w, h);
    if (region.width == 0) {
        missed++;
        fprintf(stderr, "Texture atlas is full (line %d)\n",  __LINE__);
    } else {
        w = w - 1;
        h = h - 1;
        x = region.x;
        y = region.y;
        atlas_->setRegion(x, y, w, h, ft_bitmap.buffer, ft_bitmap.pitch);

        Glyph glyph;
        glyph.charcode = charcode;
        glyph.width    = w;
        glyph.height   = h;
        glyph.outline_type = outlineType_;
        glyph.outline_thickness = outlineThickness_;
        glyph.offset_x = ft_glyph_left;
        glyph.offset_y = ft_glyph_top;
        glyph.s0       = x/(float)width;
        glyph.t0       = y/(float)height;
        glyph.s1       = (x + glyph.width)/(float)width;
        glyph.t1       = (y + glyph.height)/(float)height;

        // Discard hinting to get advance
        FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
        slot = face->glyph;
        glyph.advance_x = slot->advance.x / HRESf;
        glyph.advance_y = slot->advance.y / HRESf;

        glyphs_.push_back(glyph);

        if (outlineType_ > 0) {
            FT_Done_Glyph(ft_glyph);
        }
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    atlas_->upload();
    generateKerning();
    return missed;
}


TextureFont::Glyph* TextureFont::getGlyph(uint32_t charcode) {
    assert(atlas_);

    /* Check if charcode has been already loaded */
    for (size_t i = 0; i < glyphs_.size(); ++i) {
        auto& glyph = glyphs_[i];
        // If charcode is -1, we don't care about outline type or thickness
        if ((glyph.charcode == charcode) &&
            ((charcode == (uint32_t)(-1) ) ||
             ((glyph.outline_type == outlineType_) &&
              (glyph.outline_thickness == outlineThickness_)))) {
            return &glyph;
        }
    }

    /* charcode -1 is special : it is used for line drawing (overline,
     * underline, strikethrough) and background.
     */
    if (charcode == (uint32_t)(-1)) {
        size_t width  = atlas_->width();
        size_t height = atlas_->height();
        auto region = atlas_->getRegion(5, 5);

        Glyph glyph;
        static const signed char data[4*4*3] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                                -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
        if (region.width == 0) {
            fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
            return NULL;
        }
        atlas_->setRegion(region.x, region.y, 4, 4, data, 0);
        glyph.charcode = (uint32_t)(-1);
        glyph.s0 = (region.x+2)/(float)width;
        glyph.t0 = (region.y+2)/(float)height;
        glyph.s1 = (region.x+3)/(float)width;
        glyph.t1 = (region.y+3)/(float)height;
        glyphs_.push_back(glyph);
        return &glyphs_.back();
    }

    /* Glyph has not been already loaded */
    if (loadGlyph(charcode) == 0) {
        return &glyphs_.back();
    }
    return NULL;
}
