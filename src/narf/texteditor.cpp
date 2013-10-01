#include "narf/texteditor.h"
#include "narf/console.h"
#include "math.h"

narf::TextEditor::TextEditor() {
	homeCursor();
}

narf::TextEditor::~TextEditor() {
}

void narf::TextEditor::clear() {
	str_.clear();
	cursor = 0;
}

void narf::TextEditor::addString(const std::string &s) {
	str_.insert(cursor, s);
	cursor += 1;
}

void narf::TextEditor::setString(const std::string &s) {
	str_ = s;
	cursor = str_.size();
}

void narf::TextEditor::moveCursor(const int count) {
	cursor += count;
	cursor = std::max(0, std::min(cursor, (int)str_.size()));
}

void narf::TextEditor::backspace() {
	backspace(1);
}

void narf::TextEditor::backspace(const int count) {
	cursor = std::max(0, cursor - count);
	if (count > 0) {
		str_.erase(cursor, count);
	}
}

void narf::TextEditor::homeCursor() {
	cursor = 0;
}

void narf::TextEditor::endCursor() {
	cursor = str_.size();
}
