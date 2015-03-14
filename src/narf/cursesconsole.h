#ifndef NARF_CURSESCONSOLE_H
#define NARF_CURSESCONSOLE_H

#include "narf/console.h"

namespace narf {

	class TextEditor;
	struct CursesConsoleImpl;

	class CursesConsole : public Console {
	public:
		CursesConsole();
		~CursesConsole();
		void println(const std::string &s);
		std::string pollInput() override;

		TextEditor &getTextEditor();

		void setStatus(const std::string& status);

		std::string version() const;
	private:
		CursesConsoleImpl *impl;

		void refreshInput();
	};
};

#endif // NARF_CURSESCONSOLE_H
