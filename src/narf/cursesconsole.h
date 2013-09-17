#ifndef NARF_CURSESCONSOLE_H
#define NARF_CURSESCONSOLE_H

#include "narf/console.h"

namespace narf {

	struct CursesConsoleImpl;

	class CursesConsole : public Console {
	public:
		CursesConsole();
		~CursesConsole();
		void println(const std::string &s);
		bool pollInput();

	private:
		CursesConsoleImpl *impl;
	};
};

#endif // NARF_CURSESCONSOLE_H
