#include "narf/version.h"
#include "narf/cursesconsole.h"
#include "narf/util/path.h"


void serverLoop() {
	while (1) {
		// check for console input
		// TODO
		auto input = narf::console->pollInput();
		if (input[0]) {
			narf::console->println("Got input: '" + input + "'");
		}

		// TODO: use command parser
		if (input == "/quit") {
			break;
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

	serverLoop();

	delete narf::console;

	return 0;
}
