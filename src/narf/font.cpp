#include "narf/font.h"

#include "narf/console.h"
#include "narf/embed.h"
#include "narf/file.h"
#include "narf/utf.h"
#include "narf/util/path.h"
#include <math.h> // Need floor or something


narf::font::Font::Font() : font_(nullptr) {
	atlas_ = new TextureAtlas(512, 512, 1);
}

narf::font::Font::~Font() {
	if (font_) {
		delete font_;
	}
}


bool narf::font::Font::load(const void* data, size_t size, uint32_t pixelSize) {
	font_ = new TextureFont(atlas_, pixelSize, data, size);
	return font_ != nullptr && font_->height() != 0.0f;
}


bool narf::font::Font::load(const std::string &filename, uint32_t pixelSize) {
	if (!file_.read(filename)) {
		return false;
	}
	return load(file_.data, file_.size, pixelSize);
}


float narf::font::Font::width(const std::string &text, size_t nchars) const {
	narf::UTF8Iterator i(text);

	float width = 0;

	uint32_t prev = 0;
	size_t n = 0;
	for (const auto& c : i) {
		if (n++ >= nchars) {
			break;
		}
		auto glyph = font_->getGlyph(c);
		if (glyph) {
			if (prev) {
				width += glyph->getKerning(prev);
			}
			prev = c;
			width += glyph->advance_x;
		}
	}

	return width;
}


float narf::font::TextBuffer::width(const std::string &text, size_t nchars) const {
	return font_->width(text, nchars);
}


void narf::font::TextBuffer::print(const std::string &text, float x, float y) {
	auto black = narf::Color(0.0f, 0.0f, 0.0f);
	print(text, x, y, black);
}


void narf::font::TextBuffer::print(const std::string &text, float x, float y, const Color &color) {
	float r = color.r, g = color.g, b = color.b, a = color.a;
	float pos_x = (float)x;
	float pos_y = (float)y;
	narf::UTF8Iterator i(text);
	uint32_t prev = 0;
	for (const auto& c : i) {
		auto glyph = font_->font_->getGlyph(c);
		if (glyph != NULL) {
			if (prev) {
				pos_x += glyph->getKerning(prev);
			}
			prev = c;

			float x0  = (float)floor(pos_x + (float)glyph->offset_x);
			float y0  = (float)floor(pos_y + (float)glyph->offset_y);
			float x1  = (float)floor(x0 + (float)glyph->width);
			float y1  = (float)floor(y0 - (float)glyph->height);
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

	glBindTexture(GL_TEXTURE_2D, font_->atlas_->id());

	// TODO: move this stuff into Buffer class
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(FontVertex), (void*)offsetof(FontVertex, x));
	glTexCoordPointer(2, GL_FLOAT, sizeof(FontVertex), (void*)offsetof(FontVertex, s));
	glColorPointer(4, GL_FLOAT, sizeof(FontVertex), (void*)offsetof(FontVertex, r));

	glDrawArrays(GL_TRIANGLES, 0, (int)buffer_.count());

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	buffer_.unbind();
}


void narf::font::TextBuffer::clear() {
	buffer_.clear();
}


narf::font::EmbeddedFont::EmbeddedFont(const void* data, size_t size) :
	data_(data), size_(size) {
}


DECLARE_EMBED(DroidSansMono_ttf);

narf::font::FontManager::FontManager() {
#define ADD_EMBEDDED_FONT(name, ext) \
	embeddedFonts_.emplace(#name, EmbeddedFont{EMBED_DATA(name##_##ext), EMBED_SIZE(name##_##ext)})

	ADD_EMBEDDED_FONT(DroidSansMono, ttf);
}


narf::font::Font* narf::font::FontManager::getFont(const std::string &fontname, uint32_t pixelSize) {
	auto key = getKey(fontname, pixelSize);
	if (fonts_.count(key) > 0) {
		return fonts_[key];
	}

	auto f = new Font();
	if (embeddedFonts_.count(fontname) > 0) {
		const auto& embed = embeddedFonts_[fontname];
		console->println("Loading embedded font " + fontname);
		if (!f->load(embed.data(), embed.size(), pixelSize)) {
			delete f;
			return nullptr;
		}
	} else {
		auto basename = narf::util::appendPath(narf::util::dataDir(), fontname);
		if (!f->load(basename + ".otf", pixelSize)) {
			if (!f->load(basename + ".ttf", pixelSize)) {
				delete f;
				return nullptr;
			}
		}
	}

	fonts_.insert(std::make_pair(key, f));
	return f;
}


std::string narf::font::FontManager::getKey(const std::string& fontname, uint32_t pixelSize) const {
	return fontname + "-" + std::to_string(pixelSize);
}
