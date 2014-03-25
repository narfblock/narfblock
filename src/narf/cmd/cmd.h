// Console commands

#ifndef NARF_CMD_H
#define NARF_CMD_H

#include <map>
#include <string>

namespace narf {
	namespace cmd {

		typedef void (*ConsoleCommand)(const std::string &args);
		extern std::map<std::string, ConsoleCommand> cmds;

		void exec(const std::string &text);
	}
}

#endif // NARF_CMD_H
