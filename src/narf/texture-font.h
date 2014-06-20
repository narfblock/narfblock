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
#ifndef __TEXTURE_FONT_H__
#define __TEXTURE_FONT_H__

#include <stdlib.h>
#include <stdint.h>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_LCD_FILTER_H

#include "narf/gl/textureatlas.h"

/**
 * @file
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 */



/**
 * A structure that hold a kerning value relatively to a charcode.
 *
 * This structure cannot be used alone since the (necessary) right charcode is
 * implicitely held by the owner of this structure.
 */
typedef struct kerning_t
{
    /**
     * Left character code in the kern pair.
     */
    uint32_t charcode;

    /**
     * Kerning value (in fractional pixels).
     */
    float kerning;

} kerning_t;




/*
 * Glyph metrics:
 * --------------
 *
 *                       xmin                     xmax
 *                        |                         |
 *                        |<-------- width -------->|
 *                        |                         |
 *              |         +-------------------------+----------------- ymax
 *              |         |    ggggggggg   ggggg    |     ^        ^
 *              |         |   g:::::::::ggg::::g    |     |        |
 *              |         |  g:::::::::::::::::g    |     |        |
 *              |         | g::::::ggggg::::::gg    |     |        |
 *              |         | g:::::g     g:::::g     |     |        |
 *    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
 *              |         | g:::::g     g:::::g     |     |        |
 *              |         | g::::::g    g:::::g     |     |        |
 *              |         | g:::::::ggggg:::::g     |     |        |
 *              |         |  g::::::::::::::::g     |     |      height
 *              |         |   gg::::::::::::::g     |     |        |
 *  baseline ---*---------|---- gggggggg::::::g-----*--------      |
 *            / |         |             g:::::g     |              |
 *     origin   |         | gggggg      g:::::g     |              |
 *              |         | g:::::gg   gg:::::g     |              |
 *              |         |  g::::::ggg:::::::g     |              |
 *              |         |   gg:::::::::::::g      |              |
 *              |         |     ggg::::::ggg        |              |
 *              |         |         gggggg          |              v
 *              |         +-------------------------+----------------- ymin
 *              |                                   |
 *              |------------- advance_x ---------->|
 */



class TextureFont {
public:

    TextureFont(TextureAtlas* atlas, float ptSize, const char* filename);
    TextureFont(TextureAtlas* atlas, float ptSize, const void* memoryBase, size_t memorySize);
    ~TextureFont();

    float height() const { return height_; }

    class Glyph {
    public:
        Glyph();

        /**
         * Get the kerning between two horizontal glyphs.
         *
         * @param prevChar codepoint of the preceding glyph
         *
         * @return x kerning value
         */
        float getKerning(uint32_t prevChar) const;

        /**
         * Wide character this glyph represents
         */
        uint32_t charcode;

        /**
         * Glyph id (used for display lists)
         */
        unsigned int id;

        /**
         * Glyph's width in pixels.
         */
        size_t width;

        /**
         * Glyph's height in pixels.
         */
        size_t height;

        /**
         * Glyph's left bearing expressed in integer pixels.
         */
        int offset_x;

        /**
         * Glyphs's top bearing expressed in integer pixels.
         *
         * Remember that this is the distance from the baseline to the top-most
         * glyph scanline, upwards y coordinates being positive.
         */
        int offset_y;

        /**
         * For horizontal text layouts, this is the horizontal distance (in
         * fractional pixels) used to increment the pen position when the glyph is
         * drawn as part of a string of text.
         */
        float advance_x;

        /**
         * For vertical text layouts, this is the vertical distance (in fractional
         * pixels) used to increment the pen position when the glyph is drawn as
         * part of a string of text.
         */
        float advance_y;

        /**
         * First normalized texture coordinate (x) of top-left corner
         */
        float s0;

        /**
         * Second normalized texture coordinate (y) of top-left corner
         */
        float t0;

        /**
         * First normalized texture coordinate (x) of bottom-right corner
         */
        float s1;

        /**
         * Second normalized texture coordinate (y) of bottom-right corner
         */
        float t1;

        /**
         * A vector of kerning pairs relative to this glyph.
         */
        std::vector<kerning_t> kerning;

        /**
         * Glyph outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
         */
        int outline_type;

        /**
         * Glyph outline thickness
         */
        float outline_thickness;
    };

    Glyph* getGlyph(uint32_t charcode);

private:
    int init();
    int loadFace(float size, FT_Library* library, FT_Face* face);
    int getFaceWithSize(float size, FT_Library* library, FT_Face* face);
    int getFace(FT_Library* library, FT_Face* face);
    int getHiResFace(FT_Library* library, FT_Face* face);
    void generateKerning();
    size_t loadGlyph(uint32_t charcode);

    enum class Location {
        File = 0,
        Memory,
    };

    /**
     * Vector of glyphs contained in this font.
     */
    std::vector<Glyph> glyphs_;

    /**
     * Atlas structure to store glyphs data.
     */
    TextureAtlas* atlas_;

    /**
     * font location
     */
    Location location_;

    union {
        /**
         * Font filename, for when location == TEXTURE_FONT_FILE
         */
        char *filename_;

        /**
         * Font memory address, for when location == TEXTURE_FONT_MEMORY
         */
        struct {
            const void *base;
            size_t size;
        } memory_;
    };

    /**
     * Font size
     */
    float size_;

    /**
     * Whether to use autohint when rendering font
     */
    int hinting_;

    /**
     * Outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
     */
    int outlineType_;

    /**
     * Outline thickness
     */
    float outlineThickness_;

    /**
     * Whether to use our own lcd filter.
     */
    int filtering_;

    /**
     * Whether to use kerning if available
     */
    int kerning_;

    /**
     * LCD filter weights
     */
    unsigned char lcdWeights_[5];

    /**
     * This field is simply used to compute a default line spacing (i.e., the
     * baseline-to-baseline distance) when writing text with this font. Note
     * that it usually is larger than the sum of the ascender and descender
     * taken as absolute values. There is also no guarantee that no glyphs
     * extend above or below subsequent baselines when using this distance.
     */
    float height_;

    /**
     * This field is the distance that must be placed between two lines of
     * text. The baseline-to-baseline distance should be computed as:
     * ascender - descender + linegap
     */
    float linegap_;

    /**
     * The ascender is the vertical distance from the horizontal baseline to
     * the highest 'character' coordinate in a font face. Unfortunately, font
     * formats define the ascender differently. For some, it represents the
     * ascent of all capital latin characters (without accents), for others it
     * is the ascent of the highest accented character, and finally, other
     * formats define it as being equal to bbox.yMax.
     */
    float ascender_;

    /**
     * The descender is the vertical distance from the horizontal baseline to
     * the lowest 'character' coordinate in a font face. Unfortunately, font
     * formats define the descender differently. For some, it represents the
     * descent of all capital latin characters (without accents), for others it
     * is the ascent of the lowest accented character, and finally, other
     * formats define it as being equal to bbox.yMin. This field is negative
     * for values below the baseline.
     */
    float descender_;

    /**
     * The position of the underline line for this face. It is the center of
     * the underlining stem. Only relevant for scalable formats.
     */
    float underlinePosition_;

    /**
     * The thickness of the underline for this face. Only relevant for scalable
     * formats.
     */
    float underlineThickness_;
};

#endif /* __TEXTURE_FONT_H__ */
