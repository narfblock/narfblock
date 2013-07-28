#include "narf/font.h"
#include <math.h> // Need floor or something


narf::font::Font::Font() : font_(nullptr) {
	atlas_ = texture_atlas_new(512, 512, 1);
}

narf::font::Font::~Font() {
	texture_font_delete(font_);
}


bool narf::font::Font::load(const std::string &filename, float size) {
	font_ = texture_font_new(atlas_, filename.c_str(), size);
	return font_ != nullptr;
}


void narf::font::TextBuffer::print(const std::wstring &text, float x, float y) {
	auto black = narf::Color(0.0f, 0.0f, 0.0f);
	print(text, x, y, black);
}


void narf::font::TextBuffer::print(const std::wstring &text, float x, float y, const Color &color) {
	float r = color.r, g = color.g, b = color.b, a = color.a;
	float pos_x = (float)x;
	float pos_y = (float)y;
	for(size_t i = 0; i < text.size(); i++) {
		texture_glyph_t *glyph = texture_font_get_glyph(font_->font_, text[i]);
		if (glyph != NULL) {
			if (i > 0) {
				pos_x += texture_glyph_get_kerning(glyph, text[i - 1]);
			}

			float x0  = floor(pos_x + glyph->offset_x);
			float y0  = floor(pos_y + glyph->offset_y);
			float x1  = floor(x0 + glyph->width);
			float y1  = floor(y0 - glyph->height);
			float s0 = glyph->s0;
			float t0 = glyph->t0;
			float s1 = glyph->s1;
			float t1 = glyph->t1;
			//GLuint indices[6] = {0,1,2, 0,2,3};
			FontVertex vertices[4] = {
				{x0,y0,0,  s0,t0,  r,g,b,a},
				{x0,y1,0,  s0,t1,  r,g,b,a},
				{x1,y1,0,  s1,t1,  r,g,b,a},
				{x1,y0,0,  s1,t0,  r,g,b,a} };
			//vertex_buffer_push_back(buffer_, vertices, 4, indices, 6);
			// TODO: add index array support to Buffer
			buffer_.append(vertices[0]);
			buffer_.append(vertices[1]);
			buffer_.append(vertices[2]);
			buffer_.append(vertices[0]);
			buffer_.append(vertices[2]);
			buffer_.append(vertices[3]);
			pos_x += glyph->advance_x;
		}
	}
}


void narf::font::TextBuffer::render() {
	buffer_.upload(); // TODO: track whether this buffer has already been uploaded since last change

	buffer_.bind();

	glBindTexture(GL_TEXTURE_2D, font_->atlas_->id);

	// TODO: move this stuff into Buffer class
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(FontVertex), (void*)offsetof(FontVertex, x));
	glTexCoordPointer(2, GL_FLOAT, sizeof(FontVertex), (void*)offsetof(FontVertex, s));
	glColorPointer(4, GL_FLOAT, sizeof(FontVertex), (void*)offsetof(FontVertex, r));

	glDrawArrays(GL_TRIANGLES, 0, buffer_.count());

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	buffer_.unbind();
}


void narf::font::TextBuffer::clear() {
	buffer_.clear();
}


narf::font::Font* narf::font::FontManager::addFont(const std::string &fontname, const std::string &filename, float size) {
	if (fonts_.count(fontname) > 0) {
		return fonts_[fontname];
	}

	auto f = new Font();
	if (!f->load(filename, size)) {
		return nullptr;
	}

	fonts_.insert(std::make_pair(fontname, f));
	return f;
}


narf::font::Font* narf::font::FontManager::getFont(const std::string &fontname) {
	if (fonts_.count(fontname) > 0) {
		return fonts_[fontname];
	}
	return 0;
}
