#include <assert.h>
#include <list>

#include "narf/client/console.h"

narf::Console *narf::console;


struct narf::client::ClientConsoleImpl {
	narf::font::Font *font;
	int lineHeight;
	int x;
	int y;
	int width;
	int height;
	narf::font::TextBuffer *textBuffer;
	narf::font::TextBuffer *editBuffer;
	std::vector<std::string> text;
	narf::TextEditor editState;
};


narf::client::Console::Console() {
	impl = new ClientConsoleImpl();
	impl->font = nullptr;
	impl->textBuffer = nullptr;
	impl->editBuffer = nullptr;
}


void narf::client::Console::setFont(narf::font::Font *font, int lineHeight) {
	assert(impl->font == nullptr);
	impl->font = font;
	impl->lineHeight = lineHeight;
	impl->textBuffer = new narf::font::TextBuffer(font);
	impl->editBuffer = new narf::font::TextBuffer(font);

	// re-render current text
	update();
}


void narf::client::Console::setLocation(int x, int y, int width, int height) {
	impl->x = x;
	impl->y = y;
	impl->width = width;
	impl->height = height;
	update();
}


void narf::client::Console::setEditState(const narf::TextEditor &editor) {
	impl->editState = editor;
	update();
}


void narf::client::Console::update() {
	auto textX = static_cast<float>(impl->x);

	if (impl->textBuffer) {
		impl->textBuffer->clear();

		int y = impl->y + impl->lineHeight * 3 / 2;
		for (auto iter = impl->text.rbegin(); iter != impl->text.rend(); ++iter) {
			if (y + impl->lineHeight >= impl->height) {
				break;
			}

			auto textY = static_cast<float>(y);
			impl->textBuffer->print(*iter, textX + 1, textY - 1, narf::Color(0.0f, 0.0f, 0.0f, 1.0f));
			impl->textBuffer->print(*iter, textX, textY, narf::Color(1.0f, 1.0f, 1.0f, 1.0f));
			y += impl->lineHeight;
		}
	}

	if (impl->editBuffer) {
		impl->editBuffer->clear();

		auto editY = static_cast<float>(impl->y + impl->lineHeight / 2);
		impl->editBuffer->print(impl->editState.getString(), textX + 1, editY - 1, narf::Color(0.0f, 0.0f, 0.0f, 1.0f));
		impl->editBuffer->print(impl->editState.getString(), textX, editY, narf::Color(1.0f, 1.0f, 1.0f, 1.0f));
	}
}


void narf::client::Console::render() {
	if (impl->textBuffer) {

		// draw console background
		auto x1 = float(impl->x);
		auto y1 = float(impl->y);
		auto x2 = float(impl->x + impl->width);
		auto y2 = float(impl->y + impl->height);

		glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
		glBegin(GL_QUADS);
		glVertex2f(x1, y1);
		glVertex2f(x1, y2);
		glVertex2f(x2, y2);
		glVertex2f(x2, y1);
		glEnd();
		glColor3f(1.0f, 1.0f, 1.0f);

		impl->textBuffer->render();
	}
	if (impl->editBuffer) {
		impl->editBuffer->render();
	}
}


narf::client::Console::~Console() {
	if (impl->textBuffer) {
		delete impl->textBuffer;
	}
	delete impl;
}


void narf::client::Console::println(const std::string &s) {
	impl->text.push_back(s);
	update();

	// also print to stdout for now
	printf("%s\n", s.c_str());
}


bool narf::client::Console::pollInput() {
	return false;
}
