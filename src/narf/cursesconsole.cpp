#include "narf/cursesconsole.h"

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
};


narf::CursesConsole::CursesConsole() {
	impl = new CursesConsoleImpl();

	initscr();
	start_color();

	init_pair(1, COLOR_WHITE, COLOR_BLUE);

	cbreak(); // get one character at a time (no line buffering)
	keypad(stdscr, TRUE); // get special keys
	noecho();
	//nodelay(stdscr, TRUE); // make getch() nonblocking

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
	mvwprintw(impl->inputWin, 0, 0, "> ");

	wrefresh(impl->statusWin);
	wrefresh(impl->inputWin);
}


narf::CursesConsole::~CursesConsole() {
	endwin();
	delete impl;
}


void narf::CursesConsole::println(const std::wstring &s) {

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


bool narf::CursesConsole::pollInput() {
	int c;
	do {
		c = getch();

		// TODO: hax
#ifndef _WIN32
		if (c == KEY_RESIZE) {
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

			println(L"Resized to " + std::to_wstring(impl->numCols) + L"x" + std::to_wstring(impl->numLines));
		}
#endif
	} while (c != 'q');

	return true;
}
