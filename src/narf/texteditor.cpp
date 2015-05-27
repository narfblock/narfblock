#include "narf/texteditor.h"
#include "narf/console.h"
#include "narf/math/math.h"
#include <chrono>
#include <cstdlib>

narf::TextEditor::TextEditor() {
	homeCursor();
}

narf::TextEditor::~TextEditor() {
}

void narf::TextEditor::updated() {
	last_edited = std::chrono::system_clock::now();
}

void narf::TextEditor::clear() {
	str_.clear();
	cursor = 0;
	updated();
}

void narf::TextEditor::addString(const std::string &s) {
	str_.insert(cursor, s);
	cursor += s.length();
	updated();
}

void narf::TextEditor::setString(const std::string &s) {
	str_ = s;
	cursor = static_cast<uint32_t>(str_.size());
	updated();
}

void narf::TextEditor::moveCursor(const int count) {
	cursor = static_cast<uint32_t>(std::max(0, static_cast<int>(cursor) + count));
	cursor = std::min(cursor, static_cast<uint32_t>(str_.size()));
	updated();
}

void narf::TextEditor::delAtCursor(const int count) {
	if (count < 0) {
		moveCursor(count);
	}
	str_.erase(cursor, static_cast<uint32_t>(abs(count)));
	updated();
}

void narf::TextEditor::homeCursor() {
	cursor = 0;
	updated();
}

void narf::TextEditor::endCursor() {
	cursor = static_cast<uint32_t>(str_.size());
	updated();
}
