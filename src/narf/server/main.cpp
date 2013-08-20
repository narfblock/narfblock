#include "narf/version.h"
#include "narf/cursesconsole.h"

int main(int argc, char **argv)
{
	narf::console = new narf::CursesConsole();

	narf::console->println("Hello, world - I'm a server.");
	narf::console->println("Version: " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + std::string(VERSION_RELEASE));
	narf::console->println("Press 'q' to quit (temporary hack).");
	narf::console->pollInput();

	delete narf::console;

	return 0;
}
