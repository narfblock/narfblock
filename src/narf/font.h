#ifndef NARF_FONT_H
#define NARF_FONT_H

#include "narf/texture-font.h"

#include "narf/color.h"
#include "narf/file.h"

#include "narf/gl/gl.h"

#include <string>
#include <vector>
#include <vector>
#include <map>

namespace narf {
	namespace font {

		typedef struct {
			float x, y, z;    // position
			float s, t;       // texture
			float r, g, b, a; // color
		} FontVertex;



		class Font {
		friend class TextBuffer;
		public:
			Font();
			~Font();

			bool load(const void* data, size_t size, uint32_t pixelSize);
			bool load(const std::string &filename, uint32_t pixelSize);

			// determine width in pixels of a string in this font
			float width(const std::string &text, size_t nbytes) const;
			float height() const { return font_->height(); }

		private:
			TextureAtlas *atlas_;
			TextureFont *font_;
			MemoryFile file_;
		};


		class TextBuffer {
		public:
			TextBuffer(gl::Context& gl, const Font *font) : font_(font), buffer_(gl, GL_ARRAY_BUFFER, GL_STATIC_DRAW) {}

			void setFont(const Font* font) { font_ = font; }
			const Font* getFont() { return font_; }

			void print(const std::string &text, float x, float y);
			void print(const std::string &text, float x, float y, const Color &color);

			void render();
			void clear();

			float width(const std::string &text, size_t nbytes) const;

		private:
			const Font *font_;
			narf::gl::Buffer<FontVertex> buffer_;
		};

		class EmbeddedFont {
		public:
			EmbeddedFont() {}
			EmbeddedFont(const void* data, size_t size);

			const void* data() const { return data_; }
			size_t size() const { return size_; }

		private:
			const void* data_;
			size_t size_;
		};


		class FontManager {
		public:
			FontManager();
			Font* getFont(const std::string &fontname, uint32_t pixelSize);

			void render();
		private:
			std::string getKey(const std::string& fontname, uint32_t pixelSize) const;
			std::map<const std::string, Font*> fonts_;
			std::map<std::string, EmbeddedFont> embeddedFonts_;
		};
	}
}

#endif
