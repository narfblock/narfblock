#include "narf/version.h"
#include "narf/cursesconsole.h"
#include "narf/cmd/cmd.h"
#include "narf/util/path.h"

bool quitServerLoop = false;


void cmdQuit(const std::string &args) {
	narf::console->println("Quitting in response to user command");
	quitServerLoop = true;
}


void serverLoop() {
	while (!quitServerLoop) {
		// check for console input
		auto input = narf::console->pollInput();
		if (input != "") {
			narf::cmd::exec(input);
		}
	}
}


int main(int argc, char **argv)
{
	narf::console = new narf::CursesConsole();

	narf::console->println("Hello, world - I'm a server.");
	narf::console->println("Version: " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + std::string(VERSION_RELEASE) + "+" VERSION_REV);
	narf::console->println("Executable filename:  " + narf::util::exeName());
	narf::console->println("Executable directory: " + narf::util::exeDir());
	narf::console->println("Data directory: " + narf::util::dataDir());
	narf::console->println("Type /quit to quit.");

	narf::cmd::cmds["quit"] = cmdQuit;

	serverLoop();

	delete narf::console;

	return 0;
}
