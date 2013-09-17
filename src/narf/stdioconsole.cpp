#include <stdio.h>

#include "narf/stdioconsole.h"

narf::Console *narf::console;



narf::StdioConsole::StdioConsole() {
}


narf::StdioConsole::~StdioConsole() {
}


void narf::StdioConsole::println(const std::string &s) {
	// TODO: this is total hack
	for (auto iter = s.begin(); iter != s.end(); ++iter) {
		fputwc(*iter, stdout);
	}
	putchar('\n');
}


bool narf::StdioConsole::pollInput() {
	return true;
}