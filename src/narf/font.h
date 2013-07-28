#ifndef NARF_FONT_H
#define NARF_FONT_H

#include "freetype-gl.h"

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

		typedef union {
			float data[4];
			struct {
				float red;
				float green;
				float blue;
				float alpha;
			};
			struct {
				float r;
				float g;
				float b;
				float a;
			};
		} Color;


		class Font {
		friend class TextBuffer;
		public:
			Font();
			~Font();

			bool load(const std::string &filename, float size);

		private:
			texture_atlas_t *atlas_;
			texture_font_t *font_;
		};


		class TextBuffer {
		public:
			TextBuffer(const Font *font) : font_(font), buffer_(GL_ARRAY_BUFFER, GL_STATIC_DRAW) {}

			void print(const std::wstring &text, float x, float y);
			void print(const std::wstring &text, float x, float y, const Color &color);

			void render();
			void clear();

		private:
			const Font *font_;
			narf::gl::Buffer<FontVertex> buffer_;
		};


		class FontManager {
		public:
			FontManager() {}
			Font* addFont(const std::string &fontname, const std::string &filename, float size);
			Font* getFont(const std::string &fontname);

			void render();
		private:
			std::map<const std::string, Font*> fonts_;
		};
	}
}

#endif
