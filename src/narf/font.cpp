#include "narf/font.h"
#include <math.h> // Need floor or something

narf::font::FontManager::FontManager() {
	buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
	atlas = texture_atlas_new(512, 512, 1);
}

bool narf::font::FontManager::addFont(std::string fontname, std::string filename, size_t size) {
	if (fonts.count(fontname) > 0) {
		return false;
	}
	fonts.insert(std::make_pair(fontname, texture_font_new(atlas, filename.c_str(), size)));
	return true;
}

texture_font_t* narf::font::FontManager::getFont(std::string fontname) {
	if (fonts.count(fontname) > 0) {
		return fonts[fontname];
	}
	return 0;
}

void narf::font::FontManager::writeText(std::wstring text, std::string fontname, narf::Vector2f position) {
	writeText(text, fontname, position, {{0, 0, 0, 1}});
}

void narf::font::FontManager::writeText(std::wstring text, std::string fontname, narf::Vector2f position, narf::font::Color color) {
	float r = color.red, g = color.green, b = color.blue, a = color.alpha;
	uint32_t pos_x = position.x;
	uint32_t pos_y = position.y;
	for(size_t i = 0; i < text.size(); i++) {
		texture_glyph_t *glyph = texture_font_get_glyph(getFont(fontname), text[i]);
		if (glyph != NULL) {
			int kerning = 0;
			if (i > 0) {
				kerning = texture_glyph_get_kerning(glyph, text[i - 1]);
			}
			pos_x += kerning;
			float x0  = floor(pos_x + glyph->offset_x);
			float y0  = floor(pos_y + glyph->offset_y);
			float x1  = floor(x0 + glyph->width);
			float y1  = floor(y0 - glyph->height);
			float s0 = glyph->s0;
			float t0 = glyph->t0;
			float s1 = glyph->s1;
			float t1 = glyph->t1;
			GLuint indices[6] = {0,1,2, 0,2,3};
			narf::font::vertex_t vertices[4] = { 
				{x0,y0,0,  s0,t0,  r,g,b,a},
				{x0,y1,0,  s0,t1,  r,g,b,a},
				{x1,y1,0,  s1,t1,  r,g,b,a},
				{x1,y0,0,  s1,t0,  r,g,b,a} };
			vertex_buffer_push_back(buffer, vertices, 4, indices, 6);
			pos_x += glyph->advance_x;
		}
	}
}
