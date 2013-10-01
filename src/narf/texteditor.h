#ifndef NARF_TEXTEDITOR_H
#define NARF_TEXTEDITOR_H

#include <string>

namespace narf {
	class TextEditor {
	public:
		int cursor;
		size_t sel_length;
		TextEditor();
		~TextEditor();

		const std::string &getString() const { return str_; }

		void clear() { str_.clear(); }
		void addString(const std::string &s);
		void setString(const std::string &s);
		void moveCursor(const int count);
		void backspace();
		void backspace(const int count);
		void homeCursor();
		void endCursor();

	private:
		std::string str_;
	};
} // namespace narf

#endif // NARF_TEXTEDITOR_H
