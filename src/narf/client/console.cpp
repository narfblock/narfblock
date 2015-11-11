#include <assert.h>
#include <chrono>
#include <list>
#include <algorithm>

#include "narf/client/console.h"

struct narf::ClientConsoleImpl {

	ClientConsoleImpl() : editState() {}

	~ClientConsoleImpl() {
		if (textBuffer) {
			delete textBuffer;
		}
		if (editBuffer) {
			delete textBuffer;
		}
	}

	narf::font::Font *font;
	uint32_t lineHeight;
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
	uint32_t paddingLeft;
	uint32_t scrollback;
	narf::font::TextBuffer *textBuffer;
	narf::font::TextBuffer *editBuffer;
	std::vector<std::string> text;
	narf::TextEditor editState;
	bool editing;
	narf::ClientConsole::CursorShape cursorShape;
	narf::gl::Context* gl;
};


narf::ClientConsole::ClientConsole() {
	impl = new ClientConsoleImpl();
	impl->font = nullptr;
	impl->textBuffer = nullptr;
	impl->editBuffer = nullptr;
	impl->paddingLeft = 5; // TODO: make this configurable
	impl->cursorShape = CursorShape::Default;
	impl->gl = nullptr;
	impl->scrollback = 0;
	last_blink = std::chrono::system_clock::now();
	blink_cursor = true;
	blink_rate = 600;  // Milliseconds
}


uint32_t narf::ClientConsole::getHeightInLines() {
	// TODO: fix rendering so this is not so crazy
	return (impl->height - impl->lineHeight * 3 / 2) / impl->lineHeight;
}


void narf::ClientConsole::pageUp() {
	impl->scrollback += getHeightInLines();
	impl->scrollback = std::min(impl->scrollback, (uint32_t)impl->text.size() - getHeightInLines());
}


void narf::ClientConsole::pageDown() {
	impl->scrollback -= std::min(getHeightInLines(), impl->scrollback);
}


void narf::ClientConsole::scrollHome() {
	impl->scrollback = (uint32_t)impl->text.size() - getHeightInLines();
}


void narf::ClientConsole::scrollEnd() {
	impl->scrollback = 0;
}


void narf::ClientConsole::setGLContext(narf::gl::Context* gl) {
	impl->gl = gl;
}


narf::TextEditor &narf::ClientConsole::getTextEditor() {
	return impl->editState;
}


narf::font::Font* narf::ClientConsole::getFont() const {
	return impl->font;
}


void narf::ClientConsole::setFont(narf::font::Font *font) {
	impl->font = font;
	impl->lineHeight = (uint32_t)font->height();

	if (impl->textBuffer) {
		impl->textBuffer->setFont(font);
	} else {
		impl->textBuffer = new narf::font::TextBuffer(*impl->gl, font);
	}

	if (impl->editBuffer) {
		impl->editBuffer->setFont(font);
	} else {
		impl->editBuffer = new narf::font::TextBuffer(*impl->gl, font);
	}

	// re-render current text
	update();
}


void narf::ClientConsole::setLocation(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	if (impl->x != x ||
		impl->y != y ||
		impl->width != width ||
		impl->height != height) {
		impl->x = x;
		impl->y = y;
		impl->width = width;
		impl->height = height;
		update();
	}
}


void narf::ClientConsole::setEditState(bool editing) {
	if (editing && !impl->editing) {
		impl->editState.last_edited = std::chrono::system_clock::now();
	}
	impl->editing = editing;
	update();
}


void narf::ClientConsole::setCursorShape(CursorShape shape) {
	impl->cursorShape = shape;
}


void narf::ClientConsole::update() {
	auto textX = static_cast<float>(impl->x + impl->paddingLeft);

	if (impl->textBuffer) {
		impl->textBuffer->clear();

		uint32_t y = impl->y + impl->lineHeight * 3 / 2;
		auto scrollLines = impl->scrollback;
		for (auto iter = impl->text.rbegin(); iter != impl->text.rend(); ++iter) {
			if (scrollLines > 0) {
				scrollLines--;
				continue;
			}

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


void narf::ClientConsole::render() {
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


narf::ClientConsole::~ClientConsole() {
	delete impl;
}


void narf::ClientConsole::println(const std::string &s) {
	impl->text.push_back(s);
	scrollEnd();
	update();

	// also print to stdout for now
	printf("%s\n", s.c_str());
}


std::string narf::ClientConsole::pollInput() {
	return impl->editState.getString();
}
