#include "narf/texteditor.h"
#include "narf/console.h"
#include <chrono>
#include "math.h"

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
	cursor += 1;
	updated();
}

void narf::TextEditor::setString(const std::string &s) {
	str_ = s;
	cursor = int(str_.size());
	updated();
}

void narf::TextEditor::moveCursor(const int count) {
	cursor += count;
	cursor = std::max(0, std::min(cursor, (int)str_.size()));
	updated();
}

void narf::TextEditor::delAtCursor(const int count) {
	if (count < 0) {
		cursor = std::max(0, cursor + count);
	}
	str_.erase(cursor, std::abs(count));
	updated();
}

void narf::TextEditor::homeCursor() {
	cursor = 0;
	updated();
}

void narf::TextEditor::endCursor() {
	cursor = int(str_.size());
	updated();
}
