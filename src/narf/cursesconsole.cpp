#include "narf/cursesconsole.h"
#include "narf/texteditor.h"

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_H
#include <ncurses.h>
#elif HAVE_NCURSES_NCURSES_H
#include <ncurses/ncurses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#else
#error no curses header found
#endif


narf::Console *narf::console;


struct narf::CursesConsoleImpl {
	WINDOW *consoleWin;
	WINDOW *statusWin;
	WINDOW *inputWin;

	int numLines;
	int numCols;

	TextEditor textEditor;
};


narf::CursesConsole::CursesConsole() {
	impl = new CursesConsoleImpl();

	initscr();
	start_color();

	init_pair(1, COLOR_WHITE, COLOR_BLUE);

	cbreak(); // get one character at a time (no line buffering)
	keypad(stdscr, TRUE); // get special keys
	noecho();
	nodelay(stdscr, TRUE); // make getch() nonblocking

	getmaxyx(stdscr, impl->numLines, impl->numCols);

	refresh();

	impl->consoleWin = newwin(impl->numLines - 2, impl->numCols, 0, 0);
	scrollok(impl->consoleWin, TRUE);

	impl->statusWin = newwin(1, impl->numCols, impl->numLines - 2, 0);
	wbkgdset(impl->statusWin, COLOR_PAIR(1));
	wattron(impl->statusWin, COLOR_PAIR(1)); // TODO: Win32 pdcurses doesn't draw the full line unless this is set too?
	whline(impl->statusWin, ' ', impl->numCols);

	mvwprintw(impl->statusWin, 0, 0, "Status bar goes here");

	impl->inputWin = newwin(1, impl->numCols, impl->numLines - 1, 0);
	refreshInput();

	wrefresh(impl->statusWin);
}


void narf::CursesConsole::refreshInput() {
	mvwprintw(impl->inputWin, 0, 0, "> %s", impl->textEditor.getString().c_str());
	wclrtoeol(impl->inputWin);
	wmove(impl->inputWin, 0, impl->textEditor.cursor + 2);
	wrefresh(impl->inputWin);

}


narf::CursesConsole::~CursesConsole() {
	endwin();
	delete impl;
}


void narf::CursesConsole::println(const std::string &s) {

	// TODO: this is total hack
	for (auto iter = s.begin(); iter != s.end(); ++iter) {
		waddch(impl->consoleWin, *iter);
	}
	waddch(impl->consoleWin, '\n');

	// move cursor back to input
	wmove(impl->inputWin, 0, 2);

	wrefresh(impl->consoleWin);
	wrefresh(impl->inputWin);
}


std::string narf::CursesConsole::pollInput() {
	int c = getch();
	switch (c) {
	case ERR:
		// no input available
		return "";

	// TODO: hax
#ifndef _WIN32
	case KEY_RESIZE:
		endwin();
		refresh();
		getmaxyx(stdscr, impl->numLines, impl->numCols);
		// TODO: resize windows
		wresize(impl->consoleWin, impl->numLines - 2, impl->numCols);
		wresize(impl->statusWin, 1, impl->numCols);
		mvwin(impl->statusWin, impl->numLines - 2, 0);
		wresize(impl->inputWin, 1, impl->numCols);
		mvwin(impl->inputWin, impl->numLines - 1, 0);
		wrefresh(impl->consoleWin);

		// TODO: redraw status bar text
		wrefresh(impl->statusWin);
		wrefresh(impl->inputWin);

		println("Resized to " + std::to_string(impl->numCols) + "x" + std::to_string(impl->numLines));
		break;
#endif

	// TODO: all of these special numeric cases are bogus -
	// why doesn't curses translate them correctly?

	case KEY_ENTER:
	case 10:
	case 13: {
		auto text = impl->textEditor.getString();
		impl->textEditor.clear();
		refreshInput();
		return text;
	}

	case KEY_BACKSPACE:
	case 8:
	case 127:
		impl->textEditor.delAtCursor(-1);
		break;

	case KEY_DC: // delete character
		impl->textEditor.delAtCursor(1);
		break;

	case KEY_LEFT:
		impl->textEditor.moveCursor(-1);
		break;

	case KEY_RIGHT:
		impl->textEditor.moveCursor(1);
		break;

	case KEY_HOME:
		impl->textEditor.homeCursor();
		break;

	case KEY_END:
		impl->textEditor.endCursor();
		break;

	default:
		if (c >= ' ' && c < 127) {
			// TODO: limited to ASCII for now - do better later (UTF-8?)
			char str[] = {(char)c, '\0'};
			impl->textEditor.addString(str);
		} else {
			char str[10];
			snprintf(str, sizeof(str), "{%d}", c);
			impl->textEditor.addString(str);
		}
		break;
	}

	refreshInput();
	return "";
}


void narf::CursesConsole::setStatus(const std::string& status) {
	mvwprintw(impl->statusWin, 0, 0, status.c_str());
	wrefresh(impl->statusWin);
	refreshInput();
}
