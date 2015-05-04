/*
 * INI file load/save
 *
 * Copyright (c) 2015 Daniel Verkamp, Jessica Creighton
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
#include <cstdlib>

static bool isSpace(char c) {
	return c == ' ' || c == '\t';
}


static bool isNewLine(char c) {
	return c == '\r' || c == '\n';
}

narf::INI::Line::Line(const char* data, size_t size) {
	std::string line(data, size);
	raw = line;
	parse();
}

narf::INI::Line::Line(const std::string& line) {
	raw = line;
	parse();
}

narf::INI::Line::~Line() { }

std::string narf::INI::Line::getKey() {
	return key;
}

std::string narf::INI::Line::getValue() {
	return value;
}

std::string narf::INI::Line::getRaw() {
	return raw;
}

narf::INI::Line::Type narf::INI::Line::getType() {
	return lineType;
}

std::string narf::INI::Line::setValue(std::string newValue) {
	if (value == newValue) {
		return value;
	}
	std::string oldValue = value;
	value = newValue;
	raw = raw.substr(0, valueStartPos) + newValue + raw.substr(valueStartPos + valueLength, std::string::npos);
	return oldValue;
}


void narf::INI::Line::parse() {
	enum class State {
		BeginLine,
		EndLine,
		Comment,
		Section,
		Key,
		Equals,
		Value,
		Junk
	};
	State state = State::BeginLine;
	enum class QuoteState { None, Inside, Done };
	QuoteState quoteState = QuoteState::None;
	const char* keyStart = nullptr;
	const char* valueStart = nullptr;
	const char* sectionStart = nullptr;
	const char* chars = raw.c_str();
	size_t size = raw.size();
	error = false; // No parse errors
	bool escaping = false;
	bool valueDone = false;
	const char* valueEnd = nullptr;
	size_t i = 0;
	for (; i <= size; i++) {
		char c;
		const char* d;
		if (i == size) {
			c = '\0';
			d = &chars[size - 1];
		} else {
			c = chars[i];
			d = &chars[i];
		}
		switch (state) {
			case State::BeginLine:
				if (c == '[') {
					state = State::Section;
					lineType = Type::Section;
				} else if (isSpace(c)) {
					// eat whitespace
				} else if (isNewLine(c)) {
					// Blank line, no need to go further
					lineType = Type::Other;
				} else if (c == ';') {
					state = State::Comment;
					lineType = Type::Comment;
				} else {
					state = State::Key;
					keyStart = d;
					lineType = Type::Entry;
				};
				break;
			case State::EndLine:
				if (c == '\0' || isNewLine(c)) {
					// We done
				} else if (isSpace(c)) {
					// skip whitespace
				} else if (c == ';') {
					// Rest of line is a comment
					state = State::Comment;
				} else {
					//warn("junk at end of line");
					error = true;
					c = '\0';
				}
				break;
			case State::Comment:
				// Nothing matters
				break;
			case State::Section:
				if (sectionStart == nullptr) {
					sectionStart = d;
				}
				if (c == ']') {
					key = std::string(sectionStart, static_cast<size_t>(d - sectionStart));
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
				} else if (c == '\0' || isNewLine(c)) {
					//warn("key without value");
					error = true;
					state = State::Junk;
				} else {
					// Accumulate characters into key
					// TODO: Are there invalid key characters?
				}
				break;
				case State::Equals:
					if (isSpace(c)) {
						// Eat whitespace
					} else if (c == '=') {
						state = State::Value;
					} else {
						error = true;
						//warn("junk after key");
						state = State::Junk;
					}
					break;
				case State::Value:
					if (valueStart == nullptr) {
						if (isSpace(c)) {
							// eat whitespace between = and value
						} else {
							valueStart = d;
							valueStartPos = i;
							i--;
						}
					} else {
						if (c == '\0' || isNewLine(c) || valueDone) {
							if (escaping) {
								value.push_back('\\');
							}
							// Trim trailing whitespace. std::string doesn't have a built in trim for some reason :|
							// From here: http://stackoverflow.com/a/17976541
							if (quoteState == QuoteState::None) {
								// We weren't inside of a quote, so we should trim trailing spaces
								auto trimmedback = std::find_if_not(value.rbegin(), value.rend(), [](char c){return isSpace(c);}).base();
								value = std::string(value.begin(), trimmedback);
							}
							valueLength = (size_t)(valueEnd + 1 - valueStart);
							state = State::Comment;
						} else {
							// Accumulate any other chars into value
							if (c == ';' && quoteState != QuoteState::Inside && !escaping) {
								// We're at comment, so we're done with the value
								valueDone = true;
							} else {
								if (!isSpace(c)) {
									valueEnd = d; // Store the last non-space character so our valueLength is accurate after trimming
								}
								if (c == '\\' && !escaping) {
									escaping = true;
								} else if (escaping) {
									std::unordered_map<char, char> escapes {{'0', '\0'}, {'a', '\a'}, {'b', '\b'}, {'t', '\t'}, {'r', '\r'}, {'n', '\n'}};
									if (escapes.count(c) > 0) {
										value.push_back(escapes.at(c));
									//} else if (c == 'x') { // TODO: Hex escapes
									} else { // Catches Quote, Semicolon, and Backslash
										value.push_back(c);
									}
									escaping = false;
								} else if (c == '"') {
									switch (quoteState) {
										case QuoteState::None:
											if (valueStart == d) {
												quoteState = QuoteState::Inside;
											} else {
												error = true;
												value.push_back('"');
											}
											break;
										case QuoteState::Inside:
											quoteState = QuoteState::Done;
											break;
										case QuoteState::Done:
											error = true;
											break;
									}

								} else if (quoteState != QuoteState::Done){
									value.push_back(c);
								}
							}
						}
					}
					break;
				case State::Junk:
					// This is kind of the same as there being a comment...
					break;
		}
		if (c == '\0' || isNewLine(c)) {
			break;
		}
	}
	raw.resize(i + 1); // Truncate to just the bits we've read
}

narf::INI::File::File() { }

narf::INI::File::~File() { }

bool narf::INI::File::load(const void* data, size_t size) {
	const char* chars = static_cast<const char*>(data);
	size_t i = 0;
	std::string section;
	while (i < size) {
		narf::INI::Line line(chars + i, size - i);
		lines.push_back(line);
		i += line.getRaw().size();
		if (line.getType() == narf::INI::Line::Type::Section) {
			section = line.getKey() + ".";
		} else if (line.getType() == narf::INI::Line::Type::Entry) {
			setString(section + line.getKey(), line.getValue());
		} else {
			// Comment or blank line or something
		}
	}
	return true;
}

std::string narf::INI::File::save() {
	// It ain't pretty

	std::string section;
	std::string output;
	std::vector<std::string> savedKeys;
	// First update lines with new values and keep track of which keys exist
	for (auto &line : lines) {
		if (line.getType() == narf::INI::Line::Type::Section) {
			section = line.getKey() + ".";
		} else if (line.getType() == narf::INI::Line::Type::Entry) {
			line.setValue(getString(section + line.getKey()));
			savedKeys.push_back(section + line.getKey());
		} else {
			// Comment or blank line or something
		}
	}
	// Iterate over all values and add ones which don't exist as lines
	for (auto &pair : values_) {
		auto key = pair.first;
		if (std::count(savedKeys.begin(), savedKeys.end(), key) == 0) {
			std::string section;
			size_t lastEntry = 0;
			for (size_t i = 0; i < lines.size(); i++) {
				auto line = lines.at(i);
				if (line.getType() == narf::INI::Line::Type::Section) {
					if ((section == "" && key.find('.') == std::string::npos) ||
							(section != "" && key.find(section + ".") == 0)) {
						// If we were at global and key is at global
						// or if we got to the end of the section which matches our key
						// then insert and break
						break;
					}
					section = line.getKey();
				}
				if (line.getType() != narf::INI::Line::Type::Other) {
					lastEntry = (size_t)(i + 1);
				}
			}
			if (section != "" && key.find(section + ".") == std::string::npos) {
				// TODO: Detect which line endings to use
				lines.insert(lines.begin() + (int)lastEntry, narf::INI::Line("[" + key.substr(0, key.find(".")) + "]\n"));
				lastEntry++;
			}
			auto dotPos = key.find(".");
			if (dotPos == std::string::npos) {
				dotPos = 0;
			} else {
				dotPos++;
			}
			// TODO: Detect indentation?
			narf::INI::Line newLine((section == "" ? "" : "\t") + key.substr(dotPos, std::string::npos) + " = " + pair.second + "\n");
			lines.insert(lines.begin() + (int)lastEntry, newLine);
		}
	}
	// Iterate again for the output
	for (auto &line : lines) {
		output += line.getRaw();
	}
	return output;
}

std::string narf::INI::File::getString(const std::string& key) const {
	if (has(key)) {
		return values_.at(key);
	}
	// TODO: throw exception if key not found?
	return "";
}


void narf::INI::File::setString(const std::string& key, const std::string& value) {
	values_[key] = value;
	update(key);
}


void narf::INI::File::update(const std::string& key) {
	if (updateHandler) {
		updateHandler(key);
	}
}


bool narf::INI::File::has(const std::string& key) const {
	return values_.count(key) != 0;
}


void narf::INI::File::warn(const std::string& s) const {
	narf::console->println(s);
}


bool narf::INI::File::getBool(const std::string& key) const {
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


double narf::INI::File::getDouble(const std::string& key) const {
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


float narf::INI::File::getFloat(const std::string& key) const {
	return (float)getDouble(key);
}


int32_t narf::INI::File::getInt32(const std::string& key) const {
	auto raw = getString(key);
	char* end;
	errno = 0;
	auto value = strtol(raw.c_str(), &end, 0);
	if (errno == 0 && *end == '\0' && value <= INT32_MAX && value >= INT32_MIN) {
		return (int32_t)value;
	}
	warn(("bad int32 '" + raw + "'").c_str());
	// TODO: exception?
	return 0;
}


uint32_t narf::INI::File::getUInt32(const std::string& key) const {
	auto raw = getString(key);
	char* end;
	errno = 0;
	auto value = strtoul(raw.c_str(), &end, 0);
	if (errno == 0 && *end == '\0' && value <= UINT32_MAX) {
		return (uint32_t)value;
	}
	warn(("bad uint32 '" + raw + "'").c_str());
	// TODO: exception?
	return 0;
}


void narf::INI::File::setBool(const std::string& key, bool value) {
	setString(key, value ? "true" : "false");
}


void narf::INI::File::setDouble(const std::string& key, double value) {
	char raw[30];
	snprintf(raw, sizeof(raw), "%.17g", value);
	setString(key, raw);
}


void narf::INI::File::setFloat(const std::string& key, float value) {
	char raw[20];
	snprintf(raw, sizeof(raw), "%.9g", value);
	setString(key, raw);
}


void narf::INI::File::setInt32(const std::string& key, int32_t value) {
	char raw[20];
	snprintf(raw, sizeof(raw), "%" PRId32, value);
	setString(key, raw);
}


void narf::INI::File::initBool(const std::string& key, bool defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setBool(key, defaultValue);
	}
}


void narf::INI::File::initString(const std::string& key, const std::string& defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setString(key, defaultValue);
	}
}


void narf::INI::File::initDouble(const std::string& key, double defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setDouble(key, defaultValue);
	}
}


void narf::INI::File::initFloat(const std::string& key, float defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setFloat(key, defaultValue);
	}
}


void narf::INI::File::initInt32(const std::string& key, int32_t defaultValue) {
	if (has(key)) {
		update(key);
	} else {
		setInt32(key, defaultValue);
	}
}


std::string narf::INI::File::getString(const std::string& key, const std::string& defaultValue) const {
	return has(key) ? getString(key) : defaultValue;
}


bool narf::INI::File::getBool(const std::string& key, bool defaultValue) const {
	return has(key) ? getBool(key) : defaultValue;
}


double narf::INI::File::getDouble(const std::string& key, double defaultValue) const {
	return has(key) ? getDouble(key) : defaultValue;
}


float narf::INI::File::getFloat(const std::string& key, float defaultValue) const {
	return has(key) ? getFloat(key) : defaultValue;
}


int32_t narf::INI::File::getInt32(const std::string& key, int32_t defaultValue) const {
	return has(key) ? getInt32(key) : defaultValue;
}

uint32_t narf::INI::File::getUInt32(const std::string& key, uint32_t defaultValue) const {
	return has(key) ? getUInt32(key) : defaultValue;
}
