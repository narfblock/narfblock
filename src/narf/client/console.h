#ifndef NARF_CLIENT_CONSOLE_H
#define NARF_CLIENT_CONSOLE_H

#include <chrono>
#include "narf/console.h"
#include "narf/font.h"
#include "narf/texteditor.h"

namespace narf {
	struct ClientConsoleImpl;

	class ClientConsole : public Console {
	public:

		enum class CursorShape {
			Caret,
			Line,

			Default = Caret,
		};

		static CursorShape cursorShapeFromString(const std::string &s) {
			if (s == "caret") {
				return CursorShape::Caret;
			} else if (s == "line") {
				return CursorShape::Line;
			} else {
				return CursorShape::Default;
			}
		}

		ClientConsole();
		~ClientConsole();

		void setGLContext(gl::Context* gl);

		void println(const std::string &s);

		std::string pollInput() override;

		void setLocation(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
		narf::font::Font* getFont() const;
		void setFont(narf::font::Font *font);
		void setEditState(bool editing);
		void setCursorShape(CursorShape shape);
		void render();

		narf::TextEditor &getTextEditor();

	private:
		ClientConsoleImpl *impl;
		void update();
		std::chrono::time_point<std::chrono::system_clock> last_blink;
		bool blink_cursor;
		int blink_rate;  // Milliseconds
	};
} // namespace narf

#endif // NARF_CLIENT_CONSOLE_H
