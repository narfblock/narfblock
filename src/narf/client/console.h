#ifndef NARF_CLIENT_CONSOLE_H
#define NARF_CLIENT_CONSOLE_H

#include "narf/console.h"
#include "narf/font.h"

namespace narf {
	namespace client {

		struct ClientConsoleImpl;

		class Console : public narf::Console {
		public:
			Console();
			~Console();
			void println(const std::wstring &s);

			bool pollInput();

			void setLocation(int x, int y, int width, int height);
			void setFont(narf::font::Font *font, int lineHeight);
			void render();
		private:
			ClientConsoleImpl *impl;
			void update();
		};
	} // namespace client
} // namespace narf

#endif // NARF_CLIENT_CONSOLE_H
