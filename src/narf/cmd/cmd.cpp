#include "narf/cmd/cmd.h"
#include "narf/console.h"

std::map<std::string, narf::cmd::ConsoleCommand> narf::cmd::cmds;

void narf::cmd::exec(const std::string &text) {
	auto firstSpace = text.find_first_of(' ');
	std::string cmdStr;
	std::string params;
	if (firstSpace == std::string::npos) {
		// no space found - command only
		cmdStr = text;
		params = "";
	} else {
		// space found - split on space to get command
		cmdStr = text.substr(0, firstSpace);
		params = text.substr(firstSpace + 1);
	}

	auto cmdIter = cmds.find(cmdStr);
	if (cmdIter == cmds.end()) {
		narf::console->println("Unknown command '" + cmdStr + "'");
	} else {
		auto cmd = cmdIter->second;
		cmd(params);
	}
}
