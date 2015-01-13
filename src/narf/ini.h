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

#ifndef NARF_INI_H
#define NARF_INI_H

#include <stdint.h>

#include <string>
#include <unordered_map>
#include <functional>

namespace narf {

class IniFile {
public:
	IniFile();
	~IniFile();

	bool load(const void* data, size_t size);
	bool save(/* TODO: bytestream writer? */);

	std::string getString(const std::string& key) const;
	void setString(const std::string& key, const std::string& value);

	std::function<void(const std::string& key)> updateHandler;

	bool has(const std::string& key) const;

	bool getBool(const std::string& key) const;
	double getDouble(const std::string& key) const;
	float getFloat(const std::string& key) const;
	int32_t getInt32(const std::string& key) const;

	void setBool(const std::string& key, bool value);
	void setDouble(const std::string& key, double value);
	void setFloat(const std::string& key, float value);
	void setInt32(const std::string& key, int32_t value);

	void initString(const std::string& key, const std::string& defaultValue);
	void initBool(const std::string& key, bool defaultValue);
	void initDouble(const std::string& key, double defaultValue);
	void initFloat(const std::string& key, float defaultValue);
	void initInt32(const std::string& key, int32_t defaultValue);

	// overloads for get* with default value
	// TODO: remove these once everything uses init*
	std::string getString(const std::string& key, const std::string& defaultValue) const;
	bool getBool(const std::string& key, bool defaultValue) const;
	double getDouble(const std::string& key, double defaultValue) const;
	float getFloat(const std::string& key, float defaultValue) const;
	int32_t getInt32(const std::string& key, int32_t defaultValue) const;
private:
	// keys are stored as section.key (all sections are stored in the same map)
	std::unordered_map<std::string, std::string> values_;

	void warn(const std::string& s);
	void update(const std::string& key);
};

} // namespace narf

#endif // NARF_INI_H
