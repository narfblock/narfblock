#ifndef NARF_FONT_H
#define NARF_FONT_H

#include "freetype-gl.h"
#include "mat4.h"
#include "shader.h"
//#include "vertex-buffer.h"

#include <string>
#include <vector>
#include <vector>
#include <map>

#include "narf/vector.h"

namespace narf {
	namespace font {

		typedef struct {
			float x, y, z;    // position
			float s, t;       // texture
			float r, g, b, a; // color
		} vertex_t;

		typedef union {
			float data[4];
			struct {
				float red;
				float blue;
				float green;
				float alpha;
			};
			struct {
				float r;
				float b;
				float g;
				float a;
			};
		} Color;

		class FontManager {
			public:
				FontManager();
				bool addFont(std::string name, std::string filename, size_t size);
				texture_font_t* getFont(std::string fontname);
				void writeText(std::wstring text, std::string fontname, narf::Vector2f position);
				void writeText(std::wstring text, std::string fontname, narf::Vector2f position, narf::font::Color color);
			private:
				vertex_buffer_t* buffer;
				texture_atlas_t* atlas; 
				std::map<std::string, texture_font_t*> fonts; 
		};
	}
}

#endif
