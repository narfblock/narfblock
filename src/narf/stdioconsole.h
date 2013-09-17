#ifndef NARF_STDIOCONSOLE_H
#define NARF_STDIOCONSOLE_H

#include "narf/console.h"

namespace narf {

	class StdioConsole : public Console {
	public:
		StdioConsole();
		~StdioConsole();
		void println(const std::string &s);
		bool pollInput();
	};
};


#endif // NARF_STDIOCONSOLE_H
