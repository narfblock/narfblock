#include <assert.h>
#include <chrono>
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
	int paddingLeft;
	narf::font::TextBuffer *textBuffer;
	narf::font::TextBuffer *editBuffer;
	std::vector<std::string> text;
	narf::TextEditor editState;
	bool editing;
	narf::client::Console::CursorShape cursorShape;
};


narf::client::Console::Console() {
	impl = new ClientConsoleImpl();
	impl->font = nullptr;
	impl->textBuffer = nullptr;
	impl->editBuffer = nullptr;
	impl->paddingLeft = 5; // TODO: make this configurable
	impl->cursorShape = CursorShape::Default;
	last_blink = std::chrono::system_clock::now();
	blink_cursor = true;
	blink_rate = 600;  // Milliseconds
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


void narf::client::Console::setEditState(const narf::TextEditor &editor, bool editing) {
	impl->editState = editor;
	impl->editing = editing;
	update();
}


void narf::client::Console::setCursorShape(CursorShape shape) {
	impl->cursorShape = shape;
}


void narf::client::Console::update() {
	auto textX = static_cast<float>(impl->x + impl->paddingLeft);

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

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
		glBegin(GL_QUADS);
		glVertex2f(x1, y1);
		glVertex2f(x1, y2);
		glVertex2f(x2, y2);
		glVertex2f(x2, y1);
		glEnd();
		glPopAttrib();

		impl->textBuffer->render();
	}
	if (impl->editBuffer && impl->editing) {
		impl->editBuffer->render();

		// draw cursor
		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - impl->editState.last_edited).count() < 100) {
			blink_cursor = true;
			last_blink = now;
		} else if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_blink).count() > blink_rate) {
			last_blink = now;
			blink_cursor = !blink_cursor;
		}
		if (blink_cursor) {
			auto editY = static_cast<float>(impl->y + impl->lineHeight / 2);

			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glDisable(GL_TEXTURE_2D);
			glColor4f(1.0f, 1.0f, 1.0f, 0.7f);

			if (impl->cursorShape == CursorShape::Caret) {
				auto x1 = float(impl->x + impl->paddingLeft) + impl->editBuffer->width(impl->editState.getString(), impl->editState.cursor) - 0.5f;
				auto y1 = editY - 1.0f;

				glLineWidth(2);
				glBegin(GL_LINE_STRIP);
				glVertex2f(x1 - 4, y1 - 2);
				glVertex2f(x1, y1);
				glVertex2f(x1 + 4, y1 - 2);
				glEnd();
			} else if (impl->cursorShape == CursorShape::Line) {
				auto x1 = float(impl->x + impl->paddingLeft) + impl->editBuffer->width(impl->editState.getString(), impl->editState.cursor);
				auto y1 = editY - 1.0f;
				auto x2 = x1 + 2.0f;
				auto y2 = y1 + float(impl->lineHeight) - 1.0f;

				glBegin(GL_QUADS);
				glVertex2f(x1, y1);
				glVertex2f(x1, y2);
				glVertex2f(x2, y2);
				glVertex2f(x2, y1);
				glEnd();
			}
			glPopAttrib();
		}
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
