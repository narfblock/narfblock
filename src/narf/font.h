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

			bool load(const std::string &filename, int size);

			// determine width in pixels of a string in this font
			float width(const std::string &text, int nchars) const;
			float height() const { return font_->height(); }

		private:
			TextureAtlas *atlas_;
			TextureFont *font_;
			MemoryFile file_;
		};


		class TextBuffer {
		public:
			TextBuffer(const Font *font) : font_(font), buffer_(GL_ARRAY_BUFFER, GL_STATIC_DRAW) {}

			void setFont(const Font* font) { font_ = font; }
			const Font* getFont() { return font_; }

			void print(const std::string &text, float x, float y);
			void print(const std::string &text, float x, float y, const Color &color);

			void render();
			void clear();

			float width(const std::string &text, int nchars) const;

		private:
			const Font *font_;
			narf::gl::Buffer<FontVertex> buffer_;
		};


		class FontManager {
		public:
			FontManager() {}
			Font* getFont(const std::string &fontname, int size);

			void render();
		private:
			std::string getKey(const std::string& fontname, int size) const;
			std::map<const std::string, Font*> fonts_;
		};
	}
}

#endif
