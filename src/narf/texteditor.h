#ifndef NARF_TEXTEDITOR_H
#define NARF_TEXTEDITOR_H

#include <string>

namespace narf {
	class TextEditor {
	public:
		TextEditor();
		~TextEditor();

		const std::string &getString() const { return str_; }

		void clear() { str_.clear(); }
		void addString(const std::string &s);

	private:
		std::string str_;
	};
} // namespace narf

#endif // NARF_TEXTEDITOR_H
