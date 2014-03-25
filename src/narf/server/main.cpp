#include "narf/version.h"
#include "narf/cursesconsole.h"
#include "narf/util/path.h"


void serverLoop() {
	while (1) {
		// check for console input
		// TODO
		if (narf::console->pollInput()) {
			// bogus
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
	narf::console->println("Press 'q' to quit (temporary hack).");

	serverLoop();

	delete narf::console;

	return 0;
}
