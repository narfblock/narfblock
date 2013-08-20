#ifndef NARF_CONSOLE_H
#define NARF_CONSOLE_H

#include <string>

namespace narf {

	class Console;
	extern Console *console; // global console instance - TODO - hax

	class Console {
	public:
		virtual void println(const std::wstring &s) = 0;

		void println(const std::string &s) {
			// TODO: total hax
			std::wstring ws;
			for (auto iter = s.begin(); iter != s.end(); ++iter) {
				ws += *iter;
			}
			println(ws);
		}

		virtual bool pollInput() = 0; // TODO

	//private: // TODO
		Console() {}
		virtual ~Console() {};
	};
}

#endif // NARF_CONSOLE_H
