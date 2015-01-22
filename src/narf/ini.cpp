/*
 * INI file load/save
 *
 * Copyright (c) 2015 Daniel Verkamp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#include "narf/ini.h"

#include "narf/console.h"
#include <errno.h>

static bool isSpace(char c) {
	return c == ' ' || c == '\t';
}


static bool isNewLine(char c) {
	return c == '\r' || c == '\n';
}


narf::IniFile::IniFile() {
}


narf::IniFile::~IniFile() {
}


bool narf::IniFile::load(const void* data, size_t size) {
	enum class State {
		BeginLine, // initial state
		EndLine, // eat whitespace and newlines
		NewLine,
		Comment,
		Section,
		Key,
		Equals, // the '=' in "key = value"
		Value,
		Junk, // parse error on current line - eat everything to the end of the line
	};
	State state = State::BeginLine;
	const char* sectionStart = nullptr;
	const char* keyStart = nullptr;
	const char* valueStart = nullptr;

	std::string section, key;

	const char* chars = static_cast<const char*>(data);
	for (size_t i = 0; i <= size; i++) {
		char c;
		const char* d;
		if (i == size) {
			c = '\0';
			d = &chars[size - 1];
		} else {
			c = chars[i];
			d = &chars[i];
		}
		// TODO: think about what to do with \0 (EOF) in all states
redo:
		switch (state) {
		case State::BeginLine:
			if (c == '[') {
				state = State::Section;
			} else if (isSpace(c)) {
				// eat whitespace
			} else if (isNewLine(c)) {
				state = State::NewLine;
			} else if (c == ';') {
				state = State::Comment;
			} else {
				state = State::Key;
				keyStart = d;
			}
			break;

		case State::EndLine:
			if (isNewLine(c)) {
				state = State::NewLine;
				goto redo;
			} else if (isSpace(c)) {
				// skip whitespace
			} else {
				warn("junk at end of line");
				state = State::Junk;
			}
			break;

		case State::Comment:
			if (isNewLine(c)) {
				state = State::NewLine;
			} else {
				// skip everything else
			}
			break;

		case State::NewLine:
			if (isNewLine(c)) {
				// eat multiple newlines
			} else {
				state = State::BeginLine;
				goto redo; // reinterpret this char
			}
			break;

		case State::Section:
			if (sectionStart == nullptr) {
				sectionStart = d;
			}
			if (c == ']') {
				section = std::string(sectionStart, static_cast<size_t>(d - sectionStart));
				sectionStart = nullptr;
				state = State::EndLine;
			}
			break;

		case State::Key:
			if (isSpace(c) || c == '=') {
				if (c == '=') {
					state = State::Value;
				} else {
					state = State::Equals;
				}
				key = std::string(keyStart, static_cast<size_t>(d - keyStart));
				keyStart = nullptr;
			} else if (isNewLine(c)) {
				warn("key without value");
			} else {
				// accumulate characters into key
			}
			break;

		case State::Equals:
			if (isSpace(c)) {
				// eat whitespace
			} else if (c == '=') {
				state = State::Value;
			} else {
				warn("junk after key");
				state = State::Junk;
			}
			break;

		case State::Value:
			if (valueStart == nullptr) {
				if (isSpace(c)) {
					// eat whitespace between = and value
				} else {
					valueStart = d;
				}
			} else {
				if (isNewLine(c) || c == '\0') {
					// TODO: trim right-side whitespace
					std::string value(valueStart, static_cast<size_t>(d - valueStart));
					setString(section + "." + key, value);
					valueStart = nullptr;
					state = State::EndLine;
					goto redo;
				} else {
					// accumulate any other chars into value
				}
			}
			break;

		case State::Junk:
			sectionStart = keyStart = valueStart = nullptr;
			if (isNewLine(c)) {
				state = State::EndLine;
				goto redo;
			}
			break;
		}
	}
	return true;
}


std::string narf::IniFile::getString(const std::string& key) const {
	if (has(key)) {
		return values_.at(key);
	}
	// TODO: throw exception if key not found?
	return "";
}


void narf::IniFile::setString(const std::string& key, const std::string& value) {
	values_[key] = value;
	update(key);
}


void narf::IniFile::update(const std::string& key) {
	if (updateHandler) {
		updateHandler(key);
	}
}


bool narf::IniFile::has(const std::string& key) const {
	return values_.count(key) != 0;
}


void narf::IniFile::warn(const std::string& s) {
	narf::console->println(s);
}


bool narf::IniFile::getBool(const std::string& key) const {
	auto raw = getString(key);
	switch (raw[0]) {
	case 'y': // yes
	case 'Y':
	case 't': // true
	case 'T':
	case 'e': // enabled
	case 'E':
	case '1':
		return true;
	case 'o':
	case 'O':
		switch (raw[1]) {
		case 'n': // on
		case 'N':
			return true;
		}
		break;
	}
	return false;
}


double narf::IniFile::getDouble(const std::string& key) const {
	auto raw = getString(key);
	char* end;
	// TODO: deal with locale stuff - should this use a locale-independent format?
	errno = 0;
	auto value = strtod(raw.c_str(), &end);
	if (errno == 0 && *end == '\0') {
		return value;
	}
	// TODO: throw exception?
	return 0.0;
}


float narf::IniFile::getFloat(const std::string& key) const {
	return (float)getDouble(key);
}


int32_t narf::IniFile::getInt32(const std::string& key) const {
	auto raw = getString(key);
	char* end;
	errno = 0;
	auto value = strtol(raw.c_str(), &end, 0);
	if (errno == 0 && *end == '\0' && value <= INT32_MAX && value >= INT32_MIN) {
		return (int32_t)value;
	}
	narf::console->println("bad int32 '" + raw + "'");
	// TODO: exception?
	return 0;
}


void narf::IniFile::setBool(const std::string& key, bool value) {
	setString(key, value ? "true" : "false");
}


void narf::IniFile::setDouble(const std::string& key, double value) {
	char raw[30];
	snprintf(raw, sizeof(raw), "%.17g", value);
	setString(key, raw);
}


void narf::IniFile::setFloat(const std::string& key, float value) {
	char raw[20];
	snprintf(raw, sizeof(raw), "%.9g", value);
	setString(key, raw);
}


void narf::IniFile::setInt32(const std::string& key, int32_t value) {
	char raw[20];
	snprintf(raw, sizeof(raw), "%" PRId32, value);
	setString(key, raw);
}


void narf::IniFile::initBool(const std::string& key, bool defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setBool(key, defaultValue);
	}
}


void narf::IniFile::initString(const std::string& key, const std::string& defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setString(key, defaultValue);
	}
}


void narf::IniFile::initDouble(const std::string& key, double defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setDouble(key, defaultValue);
	}
}


void narf::IniFile::initFloat(const std::string& key, float defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setFloat(key, defaultValue);
	}
}


void narf::IniFile::initInt32(const std::string& key, int32_t defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setInt32(key, defaultValue);
	}
}


std::string narf::IniFile::getString(const std::string& key, const std::string& defaultValue) const {
	return has(key) ? getString(key) : defaultValue;
}


bool narf::IniFile::getBool(const std::string& key, bool defaultValue) const {
	return has(key) ? getBool(key) : defaultValue;
}


double narf::IniFile::getDouble(const std::string& key, double defaultValue) const {
	return has(key) ? getDouble(key) : defaultValue;
}


float narf::IniFile::getFloat(const std::string& key, float defaultValue) const {
	return has(key) ? getFloat(key) : defaultValue;
}


int32_t narf::IniFile::getInt32(const std::string& key, int32_t defaultValue) const {
	return has(key) ? getInt32(key) : defaultValue;
}
