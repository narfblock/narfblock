#include "narf/texteditor.h"
#include "narf/console.h"

narf::TextEditor::TextEditor() {
}

narf::TextEditor::~TextEditor() {
}

void narf::TextEditor::addString(const std::string &s) {
	str_ += s;
}
