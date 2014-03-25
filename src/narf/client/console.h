#ifndef NARF_CLIENT_CONSOLE_H
#define NARF_CLIENT_CONSOLE_H

#include <chrono>
#include "narf/console.h"
#include "narf/font.h"
#include "narf/texteditor.h"

namespace narf {
	namespace client {

		struct ClientConsoleImpl;

		class Console : public narf::Console {
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

			Console();
			~Console();
			void println(const std::string &s);

			std::string pollInput() override;

			void setLocation(int x, int y, int width, int height);
			void setFont(narf::font::Font *font, int lineHeight);
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
	} // namespace client
} // namespace narf

#endif // NARF_CLIENT_CONSOLE_H
