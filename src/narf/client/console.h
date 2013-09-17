#ifndef NARF_CLIENT_CONSOLE_H
#define NARF_CLIENT_CONSOLE_H

#include "narf/console.h"
#include "narf/font.h"
#include "narf/texteditor.h"

namespace narf {
	namespace client {

		struct ClientConsoleImpl;

		class Console : public narf::Console {
		public:
			Console();
			~Console();
			void println(const std::string &s);

			bool pollInput();

			void setLocation(int x, int y, int width, int height);
			void setFont(narf::font::Font *font, int lineHeight);
			void setEditState(const narf::TextEditor &editor);
			void render();
		private:
			ClientConsoleImpl *impl;
			void update();
		};
	} // namespace client
} // namespace narf

#endif // NARF_CLIENT_CONSOLE_H
